#pragma once

#include "../Cgi/CgiManager.hpp"
#include "../Config/Config.hpp"
#include "../Lib/Timeout/TimeoutManager.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Middleware/Core/MiddlewareProcessor.hpp"
#include "../Socket/Socket.hpp"
#include "../Socket/SocketsManager.hpp"
#include "Client/Client.hpp"
#include "Client/EventManager.hpp"

#include <map>

class Server {
public:
	Server(const Config &config);
	~Server();

	void run();
	void closeConnection(int clientFd);

	TimeoutManager &getTimeoutManager();
	SocketsManager &getSocketsManager();
	MiddlewareProcessor &getMainProcessor();
	CgiManager &getCgiManager();
	const Config &getConfig() const;

private:
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	Config _config;
	CgiManager _cgiManager;
	TimeoutManager _timeoutManager;
	SocketsManager _socketsManager;
	int _sigchldPipe[2];
	std::map< int, Socket * > _listenSockets;
	std::map< int, Client * > _clients;
	PipelineRouteBuilder _builder;
	MiddlewareProcessor _mainProcessor;
	EventManager _eventManager;

	void setupListenSockets();
	void setupSignalPipe();
	void handleNewConnection(int listenFd);
	void handleSigchldEvent();

	std::string getSessionId(const PipelineContext *ctx) const;
};
