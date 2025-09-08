#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "../Config/Config.hpp"
#include "../Socket/Socket.hpp"
#include "../SocketsManager/SocketsManager.hpp"

class Server {
public:
	Server();
	Server(const Config &config);
	Server(const Server &other);
	~Server();
	Server &operator=(const Server &other);

	void run();
	void addListenSocket(int port);

private:
	SocketsManager manager;
	std::map<int, Socket *> sockets;

	void handleNewConnection(int listenFd);
	void handleClientRead(int clientFd);
	void handleClientWrite(int clientFd);
	bool isRequestComplete(Socket *sock);
	void closeConnection(int clientFd);

	Config _config;
};

#endif
