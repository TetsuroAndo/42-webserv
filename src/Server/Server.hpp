#pragma once

#include "../Cgi/CgiManager.hpp"
#include "../Config/Config.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Middleware/Core/MiddlewareProcessor.hpp"
#include "../Socket/FdEventChanges.hpp"
#include "../Socket/SocketsManager.hpp"
#include "Client.hpp"
#include <map>

class Server {
public:
	Server(const Config &config);
	~Server();

	void run();

private:
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	Config _config;
	CgiManager _cgiManager;
	SocketsManager _socketsManager;
	std::map< int, Socket * > _listenSockets;
	std::map< int, Client * > _clients;

	PipelineRouteBuilder _builder;
	MiddlewareProcessor _mainProcessor;

	void applyCgiChanges();
	void setupListenSockets();
	void handleNewConnection(int listenFd);
	void handleClientRead(int clientFd);
	void handleClientWrite(int clientFd);
	void closeConnection(int clientFd);
};
