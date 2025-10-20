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

		struct addrinfo hints, *res;
		std::memset(&hints, 0, sizeof(hints));
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
					&((struct sockaddr_in *)res->ai_addr)->sin_addr,
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

			if (eventTypes & EPOLLERR || eventTypes & EPOLLHUP) {
				LOG(WARNING) << "EPOLLERR or EPOLLHUP for fd: " << fd;
				closeConnection(fd);
				continue;
			}

			if (_cgiManager.isCgiFd(fd)) {
				FdEventChanges cgiChanges =
					_cgiManager.handleEvent(fd, eventTypes);
				applyCgiChanges(cgiChanges);
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
	unsigned char *bytes =
		reinterpret_cast< unsigned char * >(&clientAddr.sin_addr.s_addr);
	snprintf(clientIp, sizeof(clientIp), "%u.%u.%u.%u", bytes[0], bytes[1],
			 bytes[2], bytes[3]);
	int clientPort = ntohs(clientAddr.sin_port);

	LOG(INFO) << "Accepted new connection" << attr("client_ip", clientIp)
			  << attr("client_port", clientPort) << attr("fd", clientFd);

	try {
		Client *client = new Client(clientFd, clientAddr, _config);
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
	char buffer[4096];

	const ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytesRead > 0) {
		ctx->recvBuffer.append(buffer, bytesRead);

		_mainProcessor.handle(*ctx);

		if (ctx->parser.isComplete() || ctx->parser.getErrorCode() != 0) {
			std::string responseStr = ResponseBuilder::build(ctx->res);
			sock->setSendBuffer(responseStr);
			_socketsManager.modifySocket(clientFd, EPOLLIN | EPOLLOUT);
			LOG(DEBUG) << "Response built and ready to send"
					   << attr("fd", clientFd);
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
		if (ctx->res.headers["Connection"] == "close") {
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
			if (ctx->res.headers["Connection"] == "close") {
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
	for (size_t i = 0; i < changes.fdsToAdd.size(); ++i) {
		_socketsManager.registerSocket(changes.fdsToAdd[i].fd,
									   changes.fdsToAdd[i].event_type);
	}
	for (size_t i = 0; i < changes.fdsToRemove.size(); ++i) {
		_socketsManager.unregisterSocket(changes.fdsToRemove[i]);
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
