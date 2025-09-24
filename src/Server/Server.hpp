#pragma once

#include "../Config/Config.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Middleware/Core/MiddlewareProcessor.hpp"
#include "../SocketsManager/SocketsManager.hpp"
#include "Client.hpp"
#include <map>

class Server {
public:
	Server();
	Server(const Config &config);
	~Server();

	void run();

private:
	Server(const Server &other);
	Server &operator=(const Server &other);

	Config _config;
	SocketsManager _manager;
	std::map<int, Socket *> _listenSockets;
	std::map<int, Client *> _clients;

	PipelineRouteBuilder _builder;
	MiddlewareProcessor _mainProcessor;

	void setupListenSockets();
	void handleNewConnection(int listenFd);
	void handleClientRead(int clientFd);
	void handleClientWrite(int clientFd);
	void closeConnection(int clientFd);
};
