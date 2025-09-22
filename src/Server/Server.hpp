#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "../Config/Config.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "../Middleware/Core/MiddlewareProcessor.hpp"
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

private:
	Config _config;
	SocketsManager _manager;
	std::map<int, Socket *> _sockets;
	std::map<int, PipelineContext *> _contexts;
	MiddlewareProcessor _mainProcessor;

	void handleNewConnection(int listenFd);
	void handleClientRead(int clientFd);
	void handleClientWrite(int clientFd);
	void closeConnection(int clientFd);
};

#endif
