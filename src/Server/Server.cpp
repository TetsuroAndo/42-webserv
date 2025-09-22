#include "Server.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

Server::Server() : _config(Config()) {
	setupListenSockets();
	PipelineRouteBuilder builder;
	builder.buildRoute(_config, &_mainProcessor);
}

Server::Server(const Config &config) : _config(config) {
	setupListenSockets();
	PipelineRouteBuilder builder;
	builder.buildRoute(_config, &_mainProcessor);
}

Server::~Server() {
	for (std::map<int, Client *>::iterator it = _clients.begin();
		 it != _clients.end(); ++it) {
		delete it->second;
	}
	for (std::map<int, Socket *>::iterator it = _listenSockets.begin();
		 it != _listenSockets.end(); ++it) {
		delete it->second;
	}
}

void Server::setupListenSockets() {
	const std::vector<Listen> &listens = _config.getListens();
	for (std::vector<Listen>::const_iterator it = listens.begin();
		 it != listens.end(); ++it) {
		int port = it->port;
		std::string interfaceAddr = it->interface;

		int listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd < 0) {
			throw std::runtime_error("socket() failed");
		}

		int flags = fcntl(listenFd, F_GETFL, 0);
		fcntl(listenFd, F_SETFL, flags | O_NONBLOCK);

		int opt = 1;
		setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(AF_INET, interfaceAddr.c_str(), &addr.sin_addr);

		if (bind(listenFd, reinterpret_cast<struct sockaddr *>(&addr),
				 sizeof(addr)) < 0) {
			close(listenFd);
			throw std::runtime_error("bind() failed for port ");
		}
		if (listen(listenFd, SOMAXCONN) < 0) {
			close(listenFd);
			throw std::runtime_error("listen() failed for port ");
		}

		Socket *sock = new Socket(listenFd, addr);
		_listenSockets[listenFd] = sock;
		_manager.registerSocket(listenFd, EPOLLIN);
		std::cout << "Listening on " << interfaceAddr << ":" << port << std::endl;
	}
}

void Server::run() {
	while (true) {
		int nEvents = _manager.wait(-1);
		if (nEvents < 0) {
			throw std::runtime_error("epoll_wait() failed");
		}

		struct epoll_event *events = _manager.getEvents();

		for (int i = 0; i < nEvents; ++i) {
			int fd = events[i].data.fd;
			uint32_t eventTypes = events[i].events;

			if ((eventTypes & EPOLLERR) || (eventTypes & EPOLLHUP)) {
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
	}
}

void Server::handleNewConnection(int listenFd) {
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFd = accept(
		listenFd, reinterpret_cast<struct sockaddr *>(&clientAddr), &clientLen);

	if (clientFd < 0) {
		return;
	}

	int flags = fcntl(clientFd, F_GETFL, 0);
	fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

	try {
		Client *client = new Client(clientFd, clientAddr, _config);
		_clients[clientFd] = client;
		_manager.registerSocket(clientFd, EPOLLIN);
	} catch (const std::bad_alloc &e) {
		std::cerr << "Failed to allocate Client object: " << e.what() << std::endl;
		close(clientFd); // Close the newly accepted socket
	} catch (const std::exception &e) {
		std::cerr << "An unexpected error occurred during client creation: " << e.what() << std::endl;
		close(clientFd); // Close the newly accepted socket
	}
}

void Server::handleClientRead(int clientFd) {
	Client *client = _clients[clientFd];
	PipelineContext *ctx = client->getContext();
	char buffer[4096];

	ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

	if (bytesRead > 0) {
		ctx->recvBuffer.append(buffer, bytesRead);
	} else {
		closeConnection(clientFd);
		return;
	}

	// Parse the request until complete or error
	ParseResult parseResult = PARSE_INCOMPLETE;
	while (parseResult == PARSE_INCOMPLETE && !ctx->recvBuffer.empty()) {
		parseResult = ctx->parser.parse(*(ctx->req), ctx->recvBuffer);
		if (parseResult == PARSE_ERROR) {
			// Set error status code in response and break
			ctx->res->setStatusCode(ctx->parser.getErrorCode());
			break;
		}
		if (parseResult == PARSE_COMPLETE) {
			break;
		}
	}

	if (ctx->parser.isComplete() || ctx->parser.getErrorCode() != 0) {
		// Only execute middleware if request is complete or parsing error occurred
		_mainProcessor.handle(*ctx);

		std::string responseStr = ResponseBuilder::build(*(ctx->res));
		if (!responseStr.empty()) {
			client->getSocket()->setSendBuffer(
				client->getSocket()->getSendBuffer() + responseStr);
		}
	}

	if (!client->getSocket()->getSendBuffer().empty()) {
		_manager.modifySocket(clientFd, EPOLLIN | EPOLLOUT);
	}
}

void Server::handleClientWrite(int clientFd) {
	Client *client = _clients[clientFd];
	Socket *sock = client->getSocket();
	const std::string &sendBuffer = sock->getSendBuffer();

	if (sendBuffer.empty()) {
		_manager.modifySocket(clientFd, EPOLLIN);
		return;
	}

	ssize_t bytesSent =
		send(clientFd, sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		sock->eraseSendBuffer(0, bytesSent);
		if (sock->getSendBuffer().empty()) {
			closeConnection(clientFd);
		}
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			closeConnection(clientFd);
		}
	}
}

void Server::closeConnection(int clientFd) {
	_manager.unregisterSocket(clientFd);
	std::map<int, Client *>::iterator it = _clients.find(clientFd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
	} else {
    }
	close(clientFd);
}
