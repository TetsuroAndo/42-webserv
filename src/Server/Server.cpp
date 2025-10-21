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
#include <stdexcept>
#include <unistd.h>
#include <vector>

Server::Server(const Config &config) : _config(config), _cgiManager(config) {
	LOG(INFO) << "Initializing server with provided configuration...";
	Logging::setupLoggers(_config);
	std::ostringstream oss;
	oss << _config;
	LOG(DEBUG) << oss.str();
	setupListenSockets();
	_builder.buildRoute(_config, &_cgiManager, &_mainProcessor);
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
			if (ptonRet < 0) {
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
		_socketsManager.registerSocket(listenFd, EPOLLIN);
		LOG(INFO) << "Listening on " << interfaceAddr << ":" << port
				  << attr("fd", listenFd);
	}
}

void Server::run() {
	LOG(INFO) << "Server is running and waiting for events.";
	time_t lastCleanTime = time(NULL);
	while (true) {
		FdEventChanges cgiChanges = _cgiManager.cleanupTimedOutWorkers();
		applyCgiChanges(cgiChanges);

		const int nEvents = _socketsManager.wait(1000);
		if (nEvents < 0) {
			LOG(FATAL) << "epoll_wait() failed: " << strerror(errno);
			throw std::runtime_error("epoll_wait() failed");
		}

		const epoll_event *events = _socketsManager.getEvents();

		for (int i = 0; i < nEvents; ++i) {
			int fd = events[i].data.fd;
			const uint32_t eventTypes = events[i].events;

			// CGI FDのイベントは先にチェック（EPOLLHUPは正常終了の可能性あり）
			if (_cgiManager.isCgiFd(fd)) {
				FdEventChanges cgiChanges =
					_cgiManager.handleEvent(fd, eventTypes);
				applyCgiChanges(cgiChanges);
			} else if (eventTypes & EPOLLERR || eventTypes & EPOLLHUP) {
				LOG(WARNING) << "EPOLLERR or EPOLLHUP for fd: " << fd;
				closeConnection(fd);
				continue;
			} else if (_listenSockets.count(fd)) {
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
			_config.getSessionCleanupIntervalSec()) {
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
		Socket *listenSocket = _listenSockets.at(listenFd);
		const int serverPort = ntohs(listenSocket->getAddr().sin_port);
		Client *client = new Client(clientFd, clientAddr, _config, serverPort);
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
	Socket *sock = client->getSocket();
	const unsigned int bufSize = ctx->conf.getIoBufferSize();
	std::vector< char > buffer(bufSize);

	const ssize_t bytesRead = recv(clientFd, &buffer[0], bufSize, 0);

	if (bytesRead > 0) {
		ctx->recvBuffer.append(&buffer[0], bytesRead);

		_mainProcessor.handle(*ctx);

		// Apply any CGI FD changes
		applyCgiChanges(ctx->changes);
		ctx->changes = FdEventChanges(); // Reset changes

		if (ctx->parser.isComplete() || ctx->parser.getErrorCode() != 0) {
			// CGIリクエストの場合は、レスポンス送信をCGI完了まで待つ
			if (!ctx->res.isCgi()) {
				std::string responseStr = ResponseBuilder::build(ctx->res);
				sock->setSendBuffer(responseStr);
				_socketsManager.modifySocket(clientFd, EPOLLIN | EPOLLOUT);
				LOG(DEBUG) << "Response built and ready to send"
						   << attr("fd", clientFd);
			} else {
				// CGI処理は非同期で継続中
				LOG(DEBUG) << "CGI request initiated, waiting for completion"
						   << attr("fd", clientFd);
			}
		}
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
}

void Server::handleClientWrite(const int clientFd) {
	Client *client = _clients[clientFd];
	Socket *sock = client->getSocket();
	PipelineContext *ctx = client->getContext(); // Get context here

	const std::string &sendBuffer = sock->getSendBuffer();
	if (sendBuffer.empty()) {
		if (ctx->res.getHeader("Connection") == "close") {
			closeConnection(clientFd);
		} else {
			_socketsManager.modifySocket(clientFd, EPOLLIN);
			ctx->reset();
		}
		return;
	}

	const ssize_t bytesSent =
		send(clientFd, sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		sock->eraseSendBuffer(0, bytesSent);
		if (!sock->getSendBuffer().empty()) {
			_socketsManager.modifySocket(clientFd, EPOLLIN | EPOLLOUT);
		} else {
			if (ctx->res.getHeader("Connection") == "close") {
				closeConnection(clientFd);
			} else {
				_socketsManager.modifySocket(clientFd, EPOLLIN);
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

void Server::applyCgiChanges(const FdEventChanges &changes) {
	LOG(DEBUG) << "applyCgiChanges called"
			   << attr("fdsToAdd", changes.fdsToAdd.size())
			   << attr("fdsToRemove", changes.fdsToRemove.size())
			   << attr("clientFdsToNotify", changes.clientFdsToNotify.size());

	for (size_t i = 0; i < changes.fdsToAdd.size(); ++i) {
		LOG(DEBUG) << "Adding CGI FD to epoll"
				   << attr("fd", changes.fdsToAdd[i].fd)
				   << attr("events", changes.fdsToAdd[i].event_type);
		_socketsManager.registerSocket(changes.fdsToAdd[i].fd,
									   changes.fdsToAdd[i].event_type);
	}
	for (size_t i = 0; i < changes.fdsToRemove.size(); ++i) {
		int fd = changes.fdsToRemove[i];
		if (fd >= 0) {
			LOG(DEBUG) << "Removing CGI FD from epoll" << attr("fd", fd);
			_socketsManager.unregisterSocket(fd);
		}
	}
	// CGI完了したクライアントへレスポンスを送信
	for (size_t i = 0; i < changes.clientFdsToNotify.size(); ++i) {
		int clientFd = changes.clientFdsToNotify[i];
		if (_clients.count(clientFd) == 0) {
			LOG(WARNING) << "Client already disconnected for CGI completion"
						 << attr("fd", clientFd);
			continue;
		}

		Client *client = _clients[clientFd];
		PipelineContext *ctx = client->getContext();
		Socket *sock = client->getSocket();

		// CGI完了のレスポンスを取得
		if (_cgiManager.isCgiComplete(clientFd, ctx->res)) {
			std::string responseStr = ResponseBuilder::build(ctx->res);
			sock->setSendBuffer(responseStr);
			_socketsManager.modifySocket(clientFd, EPOLLIN | EPOLLOUT);
			LOG(DEBUG) << "CGI response built and ready to send"
					   << attr("fd", clientFd);
		}
	}
}

void Server::closeConnection(const int clientFd) {
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
