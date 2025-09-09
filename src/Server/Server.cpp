#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "Server.hpp"

Server::Server() {
	// Configから受け取ってセッティングできるように変更したい
	int port = 8080;

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
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(listenFd, reinterpret_cast<struct sockaddr *>(&addr),
			 sizeof(addr)) < 0) {
		close(listenFd);
		throw std::runtime_error("bind() failed");
	}
	if (listen(listenFd, SOMAXCONN) < 0) {
		close(listenFd);
		throw std::runtime_error("listen() failed");
	}

	Socket *sock = new Socket(listenFd, addr);
	sock->setListen();
	_sockets[listenFd] = sock;
	_manager.registerSocket(listenFd, EPOLLIN);
}

Server::Server(const Config &config) : _config(config) {}

Server::Server(const Server & /*other*/) {}

Server::~Server() {
	for (std::map<int, Socket *>::iterator it = _sockets.begin();
		 it != _sockets.end(); ++it) {
		delete it->second;
	}
}

Server &Server::operator=(const Server & /*other*/) { return *this; }

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
				std::cerr << "epoll error on fd " << fd << std::endl;
				closeConnection(fd);
				continue;
			}

			if (eventTypes & EPOLLIN) {
				if (_sockets[fd]->isListen()) {
					handleNewConnection(fd);
				} else {
					handleClientRead(fd);
				}
			}

			if (eventTypes & EPOLLOUT) {
				handleClientWrite(fd);
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

	Socket *clientSocket = new Socket(clientFd, clientAddr);
	_sockets[clientFd] = clientSocket;
	_manager.registerSocket(clientFd, EPOLLIN);

	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
}

void Server::handleClientRead(int clientFd) {
	Socket *sock = _sockets[clientFd];
	char buffer[1024];

	while (true) {
		ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
		if (bytesRead > 0) {
			sock->appendRecvBuffer(buffer, bytesRead);
		} else if (bytesRead == 0) {
			closeConnection(clientFd);
			return;
		} else {
			break;
		}
	}
	if (sock->getRequest()->parse(sock->getRecvBuffer())) {
		// debug用のパース結果出力、提出前に消す
		sock->getRequest()->printData();
		// レスポンスを作成するmethodに置き換える
		sock->setSendBuffer("HTTP/1.0 200 OK\r\nContent-Length: "
							"13\r\nConnection: close\r\n\r\nHello, World!");
		_manager.modifySocket(clientFd, EPOLLOUT);
	}
}

void Server::handleClientWrite(int clientFd) {
	Socket *sock = _sockets[clientFd];
	if (sock->getSendBuffer().empty()) {
		_manager.modifySocket(clientFd, EPOLLIN);
		return;
	}

	const std::string &sendBuffer = sock->getSendBuffer();
	ssize_t bytesSent =
		send(clientFd, sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		sock->eraseSendBuffer(0, bytesSent);
		if (sock->getSendBuffer().empty()) {
			closeConnection(clientFd);
		}
	} else if (bytesSent == 0) {
		closeConnection(clientFd);
	} else {
		return;
	}
}

void Server::closeConnection(int clientFd) {
	_manager.unregisterSocket(clientFd);
	std::map<int, Socket *>::iterator it = _sockets.find(clientFd);
	if (it != _sockets.end()) {
		delete it->second;
		_sockets.erase(it);
	}
	close(clientFd);
}
