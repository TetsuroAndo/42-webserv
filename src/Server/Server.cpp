#include "Server.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Session/SessionManager.hpp"
#include "Logging/Logging.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <vector>

Server::Server(const Config &config)
	: _config(config), _cgiManager(config), _socketsManager(config) {
	LOG(INFO) << "Initializing server with provided configuration...";
	Logging::setupLoggers(_config);
	std::ostringstream oss;
	oss << _config;
	LOG(DEBUG) << oss.str();
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
}

void Server::applyCgiChanges() {
	while (_cgiManager.eventSize()) {
		const FdEventChange event = _cgiManager.popChange();
		try {
			switch (event.changeType) {
			case (FdChangeType_ADD):
				_socketsManager.registerSocket(
					event.fd, static_cast< uint32_t >(event.eventType));
				break;
			case (FdChangeType_REMOVE):
				_socketsManager.unregisterSocket(event.fd);
				break;
			case (FdChangeType_NOTIFY):
				_socketsManager.modifySocket(
					event.fd, static_cast< uint32_t >(event.eventType));
				break;
			}
		} catch (const std::exception &e) {
			LOG(ERROR) << "applyCgiChanges failed" << attr("fd", event.fd)
					   << attr("type", event.changeType)
					   << attr("what", e.what());
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

		Socket *sock = new Socket(_config, listenFd, addr);
		_listenSockets[listenFd] = sock;
		_socketsManager.registerSocket(listenFd, EPOLLIN);
		LOG(INFO) << "Listening on " << interfaceAddr << ":" << port
				  << attr("fd", listenFd);
	}
}

void Server::run() {
	LOG(INFO) << "Server is running and waiting for events.";
	time_t lastCleanTime = time(NULL);
	while (true) {
		const int nEvents = _socketsManager.wait(-1);
		if (nEvents < 0) {
			LOG(FATAL) << "epoll_wait() failed: " << strerror(errno);
			throw std::runtime_error("epoll_wait() failed");
		}

		// wait()から戻ったら、まず終了したCGIプロセスを回収する
		_cgiManager.cleanupFinishedWorkers();

		const epoll_event *events = _socketsManager.getEvents();

		for (int i = 0; i < nEvents; ++i) {
			int fd = events[i].data.fd;
			const uint32_t eventTypes = events[i].events;

			applyCgiChanges();

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
				if (eventTypes & EPOLLERR || eventTypes & EPOLLHUP) {
					LOG(WARNING)
						<< "EPOLLERR or EPOLLHUP for client fd: " << fd;
					closeConnection(fd);
					continue;
				}
				HttpResponse cgiRes(_config);
				if (_cgiManager.isCgiComplete(fd, cgiRes)) {
					AccessLogger::getInstance().log(
						&_clients[fd]->getContext()->req, &cgiRes,
						_clients[fd]->getIp(), _clients[fd]->getPort(),
						_clients[fd]->getContext()->session->getId());
					const std::string responseStr =
						ResponseBuilder::build(cgiRes);
					if (!responseStr.empty()) {
						_clients[fd]->getSocket()->setSendBuffer(
							_clients[fd]->getSocket()->getSendBuffer() +
							responseStr);
					}
					if (!_clients[fd]->getSocket()->getSendBuffer().empty()) {
						_socketsManager.modifySocket(fd, EPOLLIN | EPOLLOUT);
					}
				} else {
					if (eventTypes & EPOLLIN) {
						handleClientRead(fd);
					}
					if (eventTypes & EPOLLOUT) {
						handleClientWrite(fd);
					}
				}
			}
		}

		// このラウンドでCgiManagerから出た変更・通知を反映
		applyCgiChanges();

		// 完了したCGIがあれば即レスポンス組立て・送信準備
		{
			std::vector< int > clientFds;
			clientFds.reserve(_clients.size());
			for (std::map< int, Client * >::iterator it = _clients.begin();
				 it != _clients.end(); ++it) {
				clientFds.push_back(it->first);
			}
			for (size_t i = 0; i < clientFds.size(); ++i) {
				const int cfd = clientFds[i];
				if (_clients.count(cfd) == 0)
					continue;
				HttpResponse cgiRes(_config);
				if (_cgiManager.isCgiComplete(cfd, cgiRes)) {
					AccessLogger::getInstance().log(
						&_clients[cfd]->getContext()->req, &cgiRes,
						_clients[cfd]->getIp(), _clients[cfd]->getPort(),
						_clients[cfd]->getContext()->session->getId());
					const std::string responseStr =
						ResponseBuilder::build(cgiRes);
					if (!responseStr.empty()) {
						_clients[cfd]->getSocket()->setSendBuffer(
							_clients[cfd]->getSocket()->getSendBuffer() +
							responseStr);
					}
					if (!_clients[cfd]->getSocket()->getSendBuffer().empty()) {
						_socketsManager.modifySocket(cfd, EPOLLIN | EPOLLOUT);
					}
				}
			}
		}

		if (time(NULL) - lastCleanTime >
			900) { // 暫定的に15分ごとにセッションをクリア
			SessionManager::getInstance().cleanupExpiredSessions();
			lastCleanTime = time(NULL);
		}
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
	snprintf(clientIp, sizeof(clientIp), "%u.%u.%u.%u", bytes[0], bytes[1],
			 bytes[2], bytes[3]);
	const int clientPort = ntohs(clientAddr.sin_port);

	LOG(INFO) << "Accepted new connection" << attr("client_ip", clientIp)
			  << attr("client_port", clientPort) << attr("fd", clientFd);

	try {
		const Socket *listenSocket = _listenSockets.at(listenFd);
		const int listenPort = ntohs(listenSocket->getAddr().sin_port);
		Client *client =
			new Client(clientFd, clientAddr, listenPort, _cgiManager, _config);
		_clients[clientFd] = client;
		_socketsManager.registerSocket(clientFd, EPOLLIN);
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

void Server::handleClientRead(const int clientFd) {
	Client *client = _clients[clientFd];
	PipelineContext *ctx = client->getContext();
	char buffer[4096];

	const ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytesRead > 0) {
		ctx->recvBuffer.append(buffer, bytesRead);
	} else if (bytesRead == 0) {
		closeConnection(clientFd);
		return;
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "recv() failed" << attr("fd", clientFd)
					   << attr("error", strerror(errno));
			closeConnection(clientFd);
		}
		return;
	}
	_mainProcessor.handle(*ctx);

	// CGIが起動されたばかりかチェック
	if (ctx->isCgi) {
		applyCgiChanges();
		ctx->isCgi = false;
		return;
	}

	if (ctx->parser.isComplete() || ctx->parser.getErrorCode() != 0) {
		AccessLogger::getInstance().log(&ctx->req, &ctx->res, client->getIp(),
										client->getPort(),
										ctx->session->getId());
		const std::string responseStr = ResponseBuilder::build(ctx->res);
		if (!responseStr.empty()) {
			client->getSocket()->setSendBuffer(
				client->getSocket()->getSendBuffer() + responseStr);
		}
		if (!client->getSocket()->getSendBuffer().empty()) {
			_socketsManager.modifySocket(clientFd, EPOLLIN | EPOLLOUT);
		}
	}
}

void Server::handleClientWrite(const int clientFd) {
	const Client *client = _clients[clientFd];
	Socket *sock = client->getSocket();
	const std::string &sendBuffer = sock->getSendBuffer();

	if (sendBuffer.empty()) {
		_socketsManager.modifySocket(clientFd, EPOLLIN);
		return;
	}

	const ssize_t bytesSent =
		send(clientFd, sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		sock->eraseSendBuffer(0, bytesSent);
		if (sock->getSendBuffer().empty()) {
			PipelineContext *ctx = client->getContext();
			// Connectionヘッダを見て接続を閉じるか判断
			if (ctx->res.getHeader("Connection") == "close") {
				closeConnection(clientFd);
			} else {
				// Keep-Alive:
				// 接続を維持し、次のリクエストのために読み込み監視のみに戻す
				_socketsManager.modifySocket(clientFd, EPOLLIN);
				ctx->reset(_config);
			}
		}
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "send() failed" << attr("fd", clientFd)
					   << attr("error", strerror(errno));
			closeConnection(clientFd);
		}
	}
}

void Server::closeConnection(const int clientFd) {
	// 閉じる前に、CGIに紐づく処理があれば中断・後始末する
	try {
		_cgiManager.abortClient(clientFd);
	} catch (...) {
		// best-effort: ここでの失敗は致命的ではない
	}
	_socketsManager.unregisterSocket(clientFd);
	const std::map< int, Client * >::iterator it = _clients.find(clientFd);
	if (it != _clients.end()) {
		LOG(INFO) << "Closing connection"
				  << attr("client_ip", it->second->getIp())
				  << attr("fd", clientFd);
		delete it->second;
		_clients.erase(it);
	} else {
		LOG(ERROR)
			<< "Attempted to close a non-existent client connection for fd: "
			<< clientFd;
	}
	close(clientFd);
}
