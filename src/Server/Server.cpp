#include "Server.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Session/SessionManager.hpp"
#include "Client/Events/NewConnectionEvent.hpp"
#include "Client/Events/ReadEvent.hpp"
#include "Client/Events/WriteEvent.hpp"
#include "Logging/Logging.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <set>
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

std::string listenToString(const Listen &listen) {
	std::ostringstream oss;
	oss << listen.interface << ":" << listen.port;
	return oss.str();
}

const Config &selectCgiConfig(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	const Config *selected = &configs[0];
	for (size_t i = 1; i < configs.size(); ++i) {
		if (configs[i].getTimeoutSec() > selected->getTimeoutSec()) {
			selected = &configs[i];
		}
	}
	return *selected;
}

size_t resolveMaxEvents(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	size_t maxEvents = configs[0].getMaxEvents();
	for (size_t i = 1; i < configs.size(); ++i) {
		maxEvents = std::max(maxEvents, configs[i].getMaxEvents());
	}
	return maxEvents;
}

size_t resolveMaxSessionTimeout(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	size_t maxTimeout = configs[0].getSessionTimeoutSec();
	for (size_t i = 1; i < configs.size(); ++i) {
		maxTimeout = std::max(maxTimeout, configs[i].getSessionTimeoutSec());
	}
	return maxTimeout;
}

void validateListenUniqueness(const std::vector< Config > &configs) {
	std::set< std::pair< std::string, int > > seen;
	std::set< int > wildcardPorts;
	std::set< int > anyPorts;

	for (size_t i = 0; i < configs.size(); ++i) {
		const std::vector< Listen > &listens = configs[i].getListens();
		if (listens.empty()) {
			throw std::runtime_error(
				"Config error: at least one listen is required per server");
		}
		for (size_t j = 0; j < listens.size(); ++j) {
			const Listen &listen = listens[j];
			const std::pair< std::string, int > key(listen.interface,
													listen.port);
			if (seen.count(key)) {
				throw std::runtime_error("Config error: duplicate listen " +
										 listenToString(listen));
			}
			const bool isWildcard = (listen.interface == "0.0.0.0");
			if (isWildcard) {
				if (anyPorts.count(listen.port)) {
					std::ostringstream oss;
					oss << listen.port;
					throw std::runtime_error(
						"Config error: wildcard listen conflicts with "
						"existing listen on port " +
						oss.str());
				}
				wildcardPorts.insert(listen.port);
				anyPorts.insert(listen.port);
			} else {
				if (wildcardPorts.count(listen.port)) {
					std::ostringstream oss;
					oss << listen.port;
					throw std::runtime_error(
						"Config error: listen " + listenToString(listen) +
						" conflicts with wildcard listen on port " +
						oss.str());
				}
				anyPorts.insert(listen.port);
			}
			seen.insert(key);
		}
	}
}
} // namespace

Server::Server(const std::vector< Config > &configs)
	: _cgiManager(selectCgiConfig(configs)),
	  _socketsManager(resolveMaxEvents(configs)) {
	LOG(INFO) << "Initializing server with provided configuration...";
	Logging::setupLoggers(configs[0]);
	validateListenUniqueness(configs);

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

	SessionManager::getInstance().setTimeoutSec(
		static_cast< time_t >(resolveMaxSessionTimeout(configs)));
	setupListenSockets();
	LOG(INFO) << "Server initialized successfully.";
}

Server::~Server() {
	for (std::map< int, Client * >::iterator it = _clients.begin();
		 it != _clients.end(); ++it) {
		delete it->second;
	}
	for (std::map< int, Socket * >::iterator it = _listenSockets.begin();
		 it != _listenSockets.end(); ++it) {
		_eventManager.forgetFd(it->first);
		delete it->second;
	}
}

TimeoutManager &Server::getTimeoutManager() { return _timeoutManager; }
SocketsManager &Server::getSocketsManager() { return _socketsManager; }
CgiManager &Server::getCgiManager() { return _cgiManager; }

void Server::setupListenSockets() {
	for (size_t i = 0; i < _vhosts.size(); ++i) {
		const Config &config = _vhosts[i].config;
		const std::vector< Listen > &listens = config.getListens();
		for (size_t j = 0; j < listens.size(); ++j) {
			const Listen &listenConf = listens[j];
			const int port = listenConf.port;
			std::string interfaceAddr = listenConf.interface;

		int listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd < 0) {
			LOG(FATAL) << "socket() failed: " << strerror(errno);
			throw std::runtime_error("socket() failed");
		}

		const int flags = fcntl(listenFd, F_GETFL, 0);
		fcntl(listenFd, F_SETFL, flags | O_NONBLOCK);

		int opt = 1;
		setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		addrinfo hints = {}, *res;
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

		int ret = getaddrinfo(interfaceAddr.c_str(), NULL, &hints, &res);
		if (ret != 0) {
			close(listenFd);
			LOG(FATAL) << "getaddrinfo() failed for " << interfaceAddr << ": "
					   << gai_strerror(ret);
			throw std::runtime_error("getaddrinfo() failed");
		}

		std::memcpy(&addr.sin_addr,
					&reinterpret_cast< sockaddr_in * >(res->ai_addr)->sin_addr,
					sizeof(addr.sin_addr));
		freeaddrinfo(res);

		if (bind(listenFd, reinterpret_cast< sockaddr * >(&addr),
				 sizeof(addr)) < 0) {
			close(listenFd);
			LOG(FATAL) << "bind() failed for " << interfaceAddr << ":" << port
					   << ": " << strerror(errno);
			throw std::runtime_error("bind() failed");
		}
		if (listen(listenFd, SOMAXCONN) < 0) {
			close(listenFd);
			LOG(FATAL) << "listen() failed for " << interfaceAddr << ":" << port
					   << ": " << strerror(errno);
			throw std::runtime_error("listen() failed");
		}

			Socket *sock = NULL;
			try {
				sock = new Socket(config, listenFd, addr);
				_listenSockets[listenFd] = sock;
				_vhostByListenFd[listenFd] = &_vhosts[i];
				_socketsManager.registerSocket(listenFd, EPOLLIN);
				_eventManager.initFd(listenFd);
				_eventManager.addEvent(listenFd, new NewConnectionEvent(*this));
				LOG(INFO) << "Listening on " << interfaceAddr << ":" << port
						  << attr("fd", listenFd);
			} catch (const std::exception &e) {
				close(listenFd);
				delete sock;
				LOG(FATAL) << "Failed to create listen socket: " << e.what();
				throw;
			}
		}
	}
}

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
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	const int clientFd =
		accept(listenFd, reinterpret_cast< struct sockaddr * >(&clientAddr),
			   &clientLen);

	if (clientFd < 0) {
		LOG(ERROR) << "accept() failed: " << strerror(errno);
		return;
	}

	const int flags = fcntl(clientFd, F_GETFL, 0);
	fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

	char clientIp[INET_ADDRSTRLEN];
	const unsigned char *bytes =
		reinterpret_cast< unsigned char * >(&clientAddr.sin_addr.s_addr);
	makeClientIp(clientIp, sizeof(clientIp), bytes);
	const int clientPort = ntohs(clientAddr.sin_port);

	LOG(INFO) << "Accepted new connection" << attr("client_ip", clientIp)
			  << attr("client_port", clientPort) << attr("fd", clientFd);

	std::map< int, Socket * >::const_iterator it =
		_listenSockets.find(listenFd);
	if (it == _listenSockets.end()) {
		LOG(ERROR) << "Listen socket not found" << attr("fd", listenFd);
		close(clientFd);
		return;
	}
	std::map< int, VirtualHost * >::const_iterator vhostIt =
		_vhostByListenFd.find(listenFd);
	if (vhostIt == _vhostByListenFd.end()) {
		LOG(ERROR) << "Virtual host not found" << attr("fd", listenFd);
		close(clientFd);
		return;
	}

	try {
		const Socket *listenSocket = it->second;
		if (listenSocket == NULL) {
			LOG(ERROR) << "Listen socket is NULL" << attr("fd", listenFd);
			close(clientFd);
			return;
		}
		VirtualHost *vhost = vhostIt->second;
		if (vhost == NULL) {
			LOG(ERROR) << "Virtual host is NULL" << attr("fd", listenFd);
			close(clientFd);
			return;
		}
		const int listenPort = ntohs(listenSocket->getAddr().sin_port);
		Client *client =
			new Client(clientFd, clientAddr, listenPort, vhost->config,
					   vhost->mainProcessor, *this, _eventManager);
		_clients[clientFd] = client;
		_socketsManager.registerSocket(clientFd, EPOLLIN);
		_eventManager.initFd(*client);
		_eventManager.addEvent(clientFd, new ReadEvent(client));
		_eventManager.addEvent(clientFd, new WriteEvent(client));
		// 最初はヘッダ受信待ちのタイムアウトを設定
		client->updateTimeout();
	} catch (const std::bad_alloc &e) {
		LOG(ERROR) << "Failed to allocate Client object: " << e.what()
				   << attr("fd", clientFd);
		close(clientFd);
	} catch (const std::exception &e) {
		LOG(ERROR) << "Unexpected error during client creation: " << e.what()
				   << attr("fd", clientFd);
		close(clientFd);
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

std::string Server::getSessionId(const PipelineContext *ctx) const {
	if (ctx == NULL || ctx->session == NULL) {
		return "";
	}
	return ctx->session->getId();
}
