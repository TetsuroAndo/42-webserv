#include "Server.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Session/SessionManager.hpp"
#include "Bootstrap/ServerBootstrap.hpp"
#include "Client/Events/ReadEvent.hpp"
#include "Client/Events/WriteEvent.hpp"
#include "Logging/Logging.hpp"
#include "Listen/ListenKey.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

namespace {

void makeClientIp(char *clientIp, const size_t size,
				  const unsigned char bytes[4]) {
	std::ostringstream oss;
	oss << static_cast< unsigned int >(bytes[0]) << "."
		<< static_cast< unsigned int >(bytes[1]) << "."
		<< static_cast< unsigned int >(bytes[2]) << "."
		<< static_cast< unsigned int >(bytes[3]);

	std::string tmp = oss.str();
	std::strncpy(clientIp, tmp.c_str(), size);
	clientIp[size - 1] = '\0';
}

} // namespace

Server::Server(const std::vector< Config > &configs)
	: _cgiManager(ServerBootstrap::resolveCgiMaxWorkers(configs)),
	  _socketsManager(ServerBootstrap::resolveMaxEvents(configs)),
	  _listenHeaderMax(ServerBootstrap::resolveListenHeaderMax(configs)),
	  _eventManager(*this) {
	LOG(INFO) << "Initializing server with provided configuration...";
	Logging::setupLoggers(configs[0]); // TODO: 複数vhost対応
	ServerBootstrap::validateListenCompatibility(configs);

	_vhosts.reserve(configs.size());
	for (size_t i = 0; i < configs.size(); ++i) {
		VirtualHost vhost(configs[i]);
		_vhosts.push_back(vhost);
		_builder.buildRoute(_vhosts.back().config,
							&_vhosts.back().mainProcessor);

		std::ostringstream oss;
		oss << _vhosts.back().config;
		LOG(DEBUG) << "Config[" << i << "]\n" << oss.str();
	}

	_listeners.build(_vhosts, _socketsManager, _eventManager, *this);
	LOG(INFO) << "Server initialized successfully.";
}

Server::~Server() {
	for (std::map< int, Client * >::iterator it = _clients.begin();
		 it != _clients.end(); ++it) {
		delete it->second;
	}
	_listeners.forgetAll(_eventManager, _socketsManager);
}

TimeoutManager &Server::getTimeoutManager() { return _timeoutManager; }
SocketsManager &Server::getSocketsManager() { return _socketsManager; }
CgiManager &Server::getCgiManager() { return _cgiManager; }


void Server::run() {
	LOG(INFO) << "Server is running and waiting for events.";
	while (true) {
		const int timeoutMs = _timeoutManager.getNextTimeoutInterval();
		const int nEvents = _socketsManager.wait(timeoutMs);
		_timeoutManager.checkAndHandleTimeouts();
		if (nEvents < 0) {
			LOG(FATAL) << "epoll_wait() failed: " << strerror(errno);
			throw std::runtime_error("epoll_wait() failed");
		}

		// 終了したpidを拾う
		_cgiManager.cleanupFinishedWorkers();
		// タイムアウトのチェック
		_cgiManager.cleanupTimedOutWorkers();

		const epoll_event *events = _socketsManager.getEvents();

		for (int i = 0; i < nEvents; ++i) {
			const int fd = events[i].data.fd;
			const uint32_t eventTypes = events[i].events;

			_eventManager.handle(fd, eventTypes);
		}
		SessionManager::getInstance().cleanup();
	}
}

void Server::handleNewConnection(const int listenFd) {
	ListenerRegistry::AcceptedConn accepted = _listeners.acceptOnce(listenFd);
	if (!accepted.found) {
		LOG(ERROR) << "Listen socket not found" << attr("fd", listenFd);
		return;
	}
	if (accepted.fd < 0) {
        // EAGAINやEWOULDBLOCKはノンブロッキングソケットで一時的な正常状態。
        // それ以外のerrnoは異常なのでログ出力する。
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR)
				<< "accept() failed or client socket setup failed: "
				<< strerror(errno);
		}
		return;
	}

	char clientIp[INET_ADDRSTRLEN];
	const unsigned char *bytes =
		reinterpret_cast< unsigned char * >(&accepted.addr.sin_addr.s_addr);
	const int clientPort = ntohs(accepted.addr.sin_port);

	makeClientIp(clientIp, sizeof(clientIp), bytes);
	LOG(INFO) << "Accepted new connection" << attr("client_ip", clientIp)
			  << attr("client_port", clientPort) << attr("fd", accepted.fd);

	if (accepted.defaultVhostIndex >= _vhosts.size()) {
		LOG(ERROR) << "Virtual host not found" << attr("fd", listenFd);
		close(accepted.fd);
		return;
	}

	Client *client = NULL;
	try {
		VirtualHost *vhost = &_vhosts[accepted.defaultVhostIndex];

		// Host name に基づく仮想ホストの切り替えは Middleware 側で行うため、多重listen対応のため
		size_t maxHeaderBytes = vhost->config.getMaxRequestHeaderSize();
		// 接続 listen の デフォルト key
		const std::string listenKey = ListenKey::listenKeyToString(accepted.key);
		// 同 listen 単位の max があれば上書きする
		std::map< std::string, size_t >::const_iterator maxIt =
			_listenHeaderMax.find(listenKey);
		if (maxIt != _listenHeaderMax.end()) {
			maxHeaderBytes = maxIt->second;
		}

		client = new Client(accepted.fd, accepted.addr, accepted.key.port,
							*vhost, maxHeaderBytes, *this, _eventManager);

		_socketsManager.registerSocket(accepted.fd, EPOLLIN);
		_eventManager.initFd(*client);

		std::auto_ptr< ReadEvent > readEvent(new ReadEvent(client));
		_eventManager.addEvent(accepted.fd, readEvent.get());
		readEvent.release();

		std::auto_ptr< WriteEvent > writeEvent(new WriteEvent(client));
		_eventManager.addEvent(accepted.fd, writeEvent.get());
		writeEvent.release();

		// 最初はヘッダ受信待ちのタイムアウトを設定
		client->updateTimeout();
		_clients[accepted.fd] = client;
	} catch (const std::exception &e) {
		LOG(ERROR) << "Unexpected error during client creation: " << e.what()
				   << attr("fd", accepted.fd);
		if (client) {
			try {
				_timeoutManager.remove(client);
			} catch (...) {
				// Avoid suppressing the original exception during cleanup.
			}
		}
		try {
			_eventManager.forgetFd(accepted.fd);
		} catch (...) {
			// 例外処理中の二次例外で元の例外を潰さない。例外漏れによる terminate を回避
		}
		try {
			_socketsManager.unregisterSocket(accepted.fd);
		} catch (...) {
			// best-effort cleanup: ignore failures during shutdown/rollback
		}
		close(accepted.fd);
		delete client;
	}
}

void Server::closeConnection(const int clientFd) {
	// 閉じる前に、CGIに紐づく処理があれば中断・後始末する
	_cgiManager.abortClient(clientFd);
	_socketsManager.unregisterSocket(clientFd);

	// unregisterSocketをしたことで、終了通知が行かなくなるので、ここでクリアしておく
	_eventManager.forgetFd(clientFd);

	const std::map< int, Client * >::iterator it = _clients.find(clientFd);
	if (it != _clients.end()) {
		LOG(INFO) << "Closing connection"
				  << attr("client_ip", it->second->getIp())
				  << attr("fd", clientFd);
		_timeoutManager.remove(it->second);
		// ソケットを先に閉じて、相手に通知する
		close(clientFd);
		delete it->second;
		_clients.erase(it);
	} else {
		LOG(ERROR)
			<< "Attempted to close a non-existent client connection for fd: "
			<< clientFd;
		close(clientFd);
	}
}

void Server::closeFd(const int clientFd) { closeConnection(clientFd); }

std::string Server::getSessionId(const PipelineContext *ctx) const {
	if (ctx == NULL || ctx->session == NULL) {
		return "";
	}
	return ctx->session->getId();
}
