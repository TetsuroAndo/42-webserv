#include "Server.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>

Server::Server() {
	// Configから受け取ってセッティングできるように変更したい
    int port = 8080;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        throw std::runtime_error("socket() failed");
    }

    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(listen_fd);
        throw std::runtime_error("bind() failed");
    }
    if (listen(listen_fd, SOMAXCONN) < 0) {
        close(listen_fd);
        throw std::runtime_error("listen() failed");
    }

    Socket* sock = new Socket(listen_fd, addr);
    sock->setListen();
    sockets[listen_fd] = sock;
    manager.registerSocket(listen_fd, EPOLLIN);
}

//Server::Server(Config config) {
//	(void)config;
//}

Server::~Server() {
  for (std::map<int, Socket*>::iterator it = sockets.begin();
       it != sockets.end(); ++it) {
    delete it->second;
  }
}

void Server::run() {
  while (true) {
    int n_events = manager.wait(-1);
    if (n_events < 0) {
      throw std::runtime_error("epoll_wait() failed");
    }

    struct epoll_event* events = manager.getEvents();

    for (int i = 0; i < n_events; ++i) {
      int fd = events[i].data.fd;
      uint32_t event_types = events[i].events;

      if ((event_types & EPOLLERR) || (event_types & EPOLLHUP)) {
        std::cerr << "epoll error on fd " << fd << std::endl;
        closeConnection(fd);
        continue;
      }

      if (event_types & EPOLLIN) {
        if (sockets[fd]->isListen()) {
          handleNewConnection(fd);
        } else {
          handleClientRead(fd);
        }
      }

	  if (event_types & EPOLLOUT) {
	  	handleClientWrite(fd);
	  }
    }
  }
}

void Server::handleNewConnection(int listen_fd) {
  sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_fd =
      accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);

  if (client_fd < 0) {
    return;
  }

  int flags = fcntl(client_fd, F_GETFL, 0);
  fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

  Socket* client_socket = new Socket(client_fd, client_addr);
  sockets[client_fd] = client_socket;
  manager.registerSocket(client_fd, EPOLLIN);

  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
}

void Server::handleClientRead(int client_fd) {
    Socket* sock = sockets[client_fd];
    char buffer[1024];

    while (true) {
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_read > 0) {
            sock->appendRecvBuffer(buffer, bytes_read);
        } else if (bytes_read == 0) {
            closeConnection(client_fd);
            return;
        } else {
			break;
        }
    }

    if (isRequestComplete(sock)) {
		// リクエストをパースしてレスポンスを作成する
        sock->setSendBuffer("HTTP/1.0 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, World!");
        manager.modifySocket(client_fd, EPOLLOUT);
    }
}

void Server::handleClientWrite(int client_fd) {
    Socket* sock = sockets[client_fd];
    if (sock->getSendBuffer().empty()) {
        manager.modifySocket(client_fd, EPOLLIN);
        return;
    }

    const std::string& sendBuffer = sock->getSendBuffer();
    ssize_t bytes_sent = send(client_fd, sendBuffer.c_str(), sendBuffer.size(), 0);

    if (bytes_sent > 0) {
        sock->eraseSendBuffer(0, bytes_sent);
        if (sock->getSendBuffer().empty()) {
            closeConnection(client_fd);
        }
	} else if (bytes_sent == 0) {
        closeConnection(client_fd);
    } else {
		return;
    }
}

bool Server::isRequestComplete(Socket* sock) {
	// 受信データをもとに、リクエスト受信終了可否を調べる
	(void)sock;
	return true;
}

void Server::closeConnection(int client_fd) {
  manager.unregisterSocket(client_fd);
  std::map<int, Socket*>::iterator it = sockets.find(client_fd);
  if (it != sockets.end()) {
    delete it->second;
    sockets.erase(it);
  }
  close(client_fd);
}
