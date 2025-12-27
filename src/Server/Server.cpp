#include "Server.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Session/SessionManager.hpp"
#include "Client/Events/ReadEvent.hpp"
#include "Client/Events/WriteEvent.hpp"
#include "Logging/Logging.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdexcept>
#include <unistd.h>
#include <vector>

namespace {
int g_sigchldWriteFd = -1;

void sigchldHandler(int) {
	if (g_sigchldWriteFd < 0) {
		return;
	}
	const char byte = 1;
	ssize_t res = write(g_sigchldWriteFd, &byte, 1);
	(void)res;
}

void makeClientIp(char *clientIp, size_t size, const unsigned char bytes[4]) {
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

Server::Server(const Config &config)
	: _config(config), _cgiManager(config), _socketsManager(config) {
	_sigchldPipe[0] = -1;
	_sigchldPipe[1] = -1;
	LOG(INFO) << "Initializing server with provided configuration...";
	Logging::setupLoggers(_config);
	std::ostringstream oss;
	oss << _config;
	LOG(DEBUG) << oss.str();
	SessionManager::getInstance().setTimeoutSec(
		static_cast< time_t >(_config.getSessionTimeoutSec()));
	setupListenSockets();
	_builder.buildRoute(_config, &_mainProcessor);
	LOG(INFO) << "Server initialized successfully.";
}

Server::~Server() {
	for (std::map< int, Client * >::iterator it = _clients.begin();
		 it != _clients.end(); ++it) {
		delete it->second;
	}
	for (std::map< int, Socket * >::iterator it = _listenSockets.begin();
		 it != _listenSockets.end(); ++it) {
		delete it->second;
	}
	if (_sigchldPipe[0] >= 0) {
		close(_sigchldPipe[0]);
	}
	if (_sigchldPipe[1] >= 0) {
		close(_sigchldPipe[1]);
	}
	if (g_sigchldWriteFd == _sigchldPipe[1]) {
		g_sigchldWriteFd = -1;
	}
}

TimeoutManager &Server::getTimeoutManager() { return _timeoutManager; }
SocketsManager &Server::getSocketsManager() { return _socketsManager; }
MiddlewareProcessor &Server::getMainProcessor() { return _mainProcessor; }
CgiManager &Server::getCgiManager() { return _cgiManager; }
const Config &Server::getConfig() const { return _config; }

void Server::applyCgiChanges() {
	FdEventChange event;
	while (_cgiManager.sizeAddEvent() || _cgiManager.sizeRemoveEvent() ||
		   _cgiManager.sizeNotifyEvent()) {
		try {
			while (_cgiManager.sizeAddEvent()) {
				event = _cgiManager.popAddChange();
				_socketsManager.registerSocket(
					event.fd, static_cast< uint32_t >(event.eventType));
			}
			while (_cgiManager.sizeRemoveEvent()) {
				event = _cgiManager.popRemoveChange();
				_socketsManager.unregisterSocket(event.fd);
			}
			while (_cgiManager.sizeNotifyEvent()) {
				event = _cgiManager.popNotifyChange();
				_socketsManager.modifySocket(
					event.fd, static_cast< uint32_t >(event.eventType));
			}
		} catch (const std::runtime_error &e) {
			LOG(ERROR) << "applyCgiChanges: socket operation failed"
					   << attr("fd", event.fd) << attr("what", e.what());
		} catch (const std::exception &e) {
			LOG(ERROR) << "applyCgiChanges: unexpected exception"
					   << attr("fd", event.fd) << attr("what", e.what());
		}
	}
}

void Server::setupListenSockets() {
	const std::vector< Listen > &listens = _config.getListens();
	for (std::vector< Listen >::const_iterator it = listens.begin();
		 it != listens.end(); ++it) {
		const int port = it->port;
		std::string interfaceAddr = it->interface;

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
			sock = new Socket(_config, listenFd, addr);
			_listenSockets[listenFd] = sock;
			_socketsManager.registerSocket(listenFd, EPOLLIN);
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

void Server::run() {
	setupSignalPipe();
	LOG(INFO) << "Server is running and waiting for events.";
	while (true) {
		const int timeoutMs = _timeoutManager.getNextTimeoutInterval();
		const int nEvents = _socketsManager.wait(timeoutMs);
		_timeoutManager.checkAndHandleTimeouts();
		if (nEvents < 0) {
			LOG(FATAL) << "epoll_wait() failed: " << strerror(errno);
			throw std::runtime_error("epoll_wait() failed");
		}

		// wait()から戻ったら、まず終了したCGIプロセスを回収する
		_cgiManager.cleanupFinishedWorkers();
		// タイムアウトのチェックも行う（イベント駆動で完了通知を積む）
		_cgiManager.cleanupTimedOutWorkers();

		const epoll_event *events = _socketsManager.getEvents();

		for (int i = 0; i < nEvents; ++i) {
			int fd = events[i].data.fd;
			const uint32_t eventTypes = events[i].events;

			applyCgiChanges();

			if (fd == _sigchldPipe[0]) {
				handleSigchldEvent();
				continue;
			}

			// CGIのFDを優先的に処理する
			if (_cgiManager.isCgiFd(fd)) {
				uint32_t ev = eventTypes;
				if (eventTypes & EPOLLHUP) {
					ev |= EPOLLIN; // EOF処理のため
				}
				_cgiManager.handleEvent(fd, ev);
				continue;
			}

			if (_listenSockets.count(fd)) {
				handleNewConnection(fd);
			} else if (_clients.count(fd)) {
				_eventManager.handle(fd, eventTypes);

				HttpResponse cgiRes(_config);
				if (_cgiManager.isCgiComplete(fd, cgiRes)) {
					AccessLogger::getInstance().log(
						&_clients[fd]->getContext().req, &cgiRes,
						_clients[fd]->getIp(), _clients[fd]->getPort(),
						getSessionId(&_clients[fd]->getContext()));
					const std::string responseStr =
						ResponseBuilder::build(cgiRes);
					if (!responseStr.empty()) {
						_clients[fd]->getSocket().setSendBuffer(
							_clients[fd]->getSocket().getSendBuffer() +
							responseStr);
					}
					if (!_clients[fd]->getSocket().getSendBuffer().empty()) {
						_socketsManager.modifySocket(fd, EPOLLIN | EPOLLOUT);
					}
				} else {
					// if (eventTypes & EPOLLIN) {
					// 	_clients[fd]->handleReadEvent();
					// }
					// //タイムアウトでクライアントが削除された可能性があるため再度チェック
					// if (_clients.count(fd) && (eventTypes & EPOLLOUT)) {
					// 	_clients[fd]->handleWriteEvent();
					// }
				}
			}
		}

		// このラウンドでCgiManagerから出た変更・通知を反映
		applyCgiChanges();

		// 完了通知が来たクライアントのみレスポンス組立て・送信準備
		while (_cgiManager.sizeCompletedClientFd()) {
			const int cfd = _cgiManager.popCompletedClientFd();
			if (_clients.count(cfd) == 0)
				continue;
			HttpResponse cgiRes(_config);
			if (_cgiManager.isCgiComplete(cfd, cgiRes)) {
				AccessLogger::getInstance().log(
					&_clients[cfd]->getContext().req, &cgiRes,
					_clients[cfd]->getIp(), _clients[cfd]->getPort(),
					getSessionId(&_clients[cfd]->getContext()));
				const std::string responseStr = ResponseBuilder::build(cgiRes);
				if (!responseStr.empty()) {
					_clients[cfd]->getSocket().setSendBuffer(
						_clients[cfd]->getSocket().getSendBuffer() +
						responseStr);
				}
				if (!_clients[cfd]->getSocket().getSendBuffer().empty()) {
					_socketsManager.modifySocket(cfd, EPOLLIN | EPOLLOUT);
				}
			}
		}

		SessionManager::getInstance().cleanupIfNeeded();
	}
}

void Server::setupSignalPipe() {
	if (_sigchldPipe[0] >= 0 || _sigchldPipe[1] >= 0) {
		return;
	}

	if (pipe(_sigchldPipe) < 0) {
		throw std::runtime_error("pipe(SIGCHLD) failed");
	}

	int flags = fcntl(_sigchldPipe[0], F_GETFL, 0);
	if (flags < 0 || fcntl(_sigchldPipe[0], F_SETFL, flags | O_NONBLOCK) < 0) {
		close(_sigchldPipe[0]);
		close(_sigchldPipe[1]);
		_sigchldPipe[0] = -1;
		_sigchldPipe[1] = -1;
		throw std::runtime_error("fcntl(SIGCHLD pipe read) failed");
	}
	flags = fcntl(_sigchldPipe[1], F_GETFL, 0);
	if (flags < 0 || fcntl(_sigchldPipe[1], F_SETFL, flags | O_NONBLOCK) < 0) {
		close(_sigchldPipe[0]);
		close(_sigchldPipe[1]);
		_sigchldPipe[0] = -1;
		_sigchldPipe[1] = -1;
		throw std::runtime_error("fcntl(SIGCHLD pipe write) failed");
	}

	g_sigchldWriteFd = _sigchldPipe[1];
	if (signal(SIGCHLD, sigchldHandler) == SIG_ERR) {
		close(_sigchldPipe[0]);
		close(_sigchldPipe[1]);
		_sigchldPipe[0] = -1;
		_sigchldPipe[1] = -1;
		g_sigchldWriteFd = -1;
		throw std::runtime_error("signal(SIGCHLD) failed");
	}

	try {
		_socketsManager.registerSocket(_sigchldPipe[0], EPOLLIN);
	} catch (...) {
		close(_sigchldPipe[0]);
		close(_sigchldPipe[1]);
		_sigchldPipe[0] = -1;
		_sigchldPipe[1] = -1;
		g_sigchldWriteFd = -1;
		throw;
	}
}

void Server::handleSigchldEvent() {
	char buffer[64];
	while (true) {
		const ssize_t bytes = read(_sigchldPipe[0], buffer, sizeof(buffer));
		if (bytes > 0) {
			continue;
		}
		if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			break;
		}
		if (bytes < 0) {
			LOG(ERROR) << "read(SIGCHLD pipe) failed"
					   << attr("error", strerror(errno));
		}
		break;
	}
	_cgiManager.cleanupFinishedWorkers();
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

	try {
		const Socket *listenSocket = it->second;
		if (listenSocket == NULL) {
			LOG(ERROR) << "Listen socket is NULL" << attr("fd", listenFd);
			close(clientFd);
			return;
		}
		const int listenPort = ntohs(listenSocket->getAddr().sin_port);
		Client *client =
			new Client(clientFd, clientAddr, listenPort, *this, _eventManager);
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
