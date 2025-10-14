#include "Server.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Session/SessionManager.hpp"
#include "Logging/Logging.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

Server::Server() : _config(Config()) {
	LOG(INFO) << "Initializing server with default configuration...";
	Logging::setupLoggers(_config);
	std::ostringstream oss;
	oss << _config;
	LOG(DEBUG) << oss.str();
	setupListenSockets();
	_builder.buildRoute(_config, &_mainProcessor);
	LOG(INFO) << "Server initialized successfully.";
}

Server::Server(const Config &config) : _config(config) {
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
		int ptonRet = inet_pton(AF_INET, interfaceAddr.c_str(), &addr.sin_addr);
		if (ptonRet <= 0) {
			close(listenFd);
			if (ptonRet == 0) {
				LOG(FATAL) << "Invalid IP address format: " << interfaceAddr;
				throw std::runtime_error("Invalid IP address format");     
			}
			if(ptonRet < 0) {
				LOG(FATAL) << "inet_pton() failed: " << strerror(errno);
				throw std::runtime_error("inet_pton() failed");   
			}
		}

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

		Socket *sock = new Socket(listenFd, addr);
		_listenSockets[listenFd] = sock;
		_manager.registerSocket(listenFd, EPOLLIN);
		LOG(INFO) << "Listening on " << interfaceAddr << ":" << port
				  << attr("fd", listenFd);
	}
}

void Server::run() {
	LOG(INFO) << "Server is running and waiting for events.";
	time_t lastCleanTime = time(NULL);
	while (true) {
		const int nEvents = _manager.wait(-1);
		if (nEvents < 0) {
			LOG(FATAL) << "epoll_wait() failed: " << strerror(errno);
			throw std::runtime_error("epoll_wait() failed");
		}

		const epoll_event *events = _manager.getEvents();

		for (int i = 0; i < nEvents; ++i) {
			int fd = events[i].data.fd;
			const uint32_t eventTypes = events[i].events;

			if (eventTypes & EPOLLERR || eventTypes & EPOLLHUP) {
				LOG(WARNING) << "EPOLLERR or EPOLLHUP for fd: " << fd;
				closeConnection(fd);
				continue;
			}

			if (_listenSockets.count(fd)) {
				handleNewConnection(fd);
			} else if (_clients.count(fd)) {
				if (eventTypes & EPOLLIN) {
					handleClientRead(fd);
				}
				if (eventTypes & EPOLLOUT) {
					handleClientWrite(fd);
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
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, sizeof(clientIp));
	int clientPort = ntohs(clientAddr.sin_port);

	LOG(INFO) << "Accepted new connection" << attr("client_ip", clientIp)
			  << attr("client_port", clientPort) << attr("fd", clientFd);

	try {
		Client *client = new Client(clientFd, clientAddr, _config);
		_clients[clientFd] = client;
		_manager.registerSocket(clientFd, EPOLLIN);
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

	if (ctx->parser.isComplete() || ctx->parser.getErrorCode() != 0) {
		AccessLogger::getInstance().log(ctx->req, ctx->res, client->getIp(),
										client->getPort(),
										ctx->session->getId());
		const std::string responseStr = ResponseBuilder::build(*ctx->res);
		if (!responseStr.empty()) {
			client->getSocket()->setSendBuffer(
				client->getSocket()->getSendBuffer() + responseStr);
		}
		if (!client->getSocket()->getSendBuffer().empty()) {
			_manager.modifySocket(clientFd, EPOLLIN | EPOLLOUT);
		}
	}
}

void Server::handleClientWrite(const int clientFd) {
	Client *client = _clients[clientFd];
	Socket *sock = client->getSocket();
	const std::string &sendBuffer = sock->getSendBuffer();

	if (sendBuffer.empty()) {
		_manager.modifySocket(clientFd, EPOLLIN);
		return;
	}

	const ssize_t bytesSent =
		send(clientFd, sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		sock->eraseSendBuffer(0, bytesSent);
		if (sock->getSendBuffer().empty()) {
			PipelineContext *ctx = client->getContext();
			// Connectionヘッダを見て接続を閉じるか判断
			if (ctx->res->getHeader("Connection") == "close") {
				closeConnection(clientFd);
			} else {
				// Keep-Alive: 接続を維持し、次のリクエストのために読み込み監視のみに戻す
				_manager.modifySocket(clientFd, EPOLLIN);
				ctx->reset(); 
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
	_manager.unregisterSocket(clientFd);
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
