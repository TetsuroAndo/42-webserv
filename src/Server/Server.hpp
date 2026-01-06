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
#include <vector>

class Server {
public:
	Server(const std::vector< Config > &configs);
	~Server();

	void run();
	void closeConnection(int clientFd);

	TimeoutManager &getTimeoutManager();
	SocketsManager &getSocketsManager();
	CgiManager &getCgiManager();

private:
	struct VirtualHost {
		Config config;
		MiddlewareProcessor mainProcessor;
		VirtualHost(const Config &conf) : config(conf), mainProcessor() {}
	};

	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	std::vector< VirtualHost > _vhosts;
	std::map< int, VirtualHost * > _vhostByListenFd;
	CgiManager _cgiManager;
	TimeoutManager _timeoutManager;
	SocketsManager _socketsManager;
	int _sigchldPipe[2];
	std::map< int, Socket * > _listenSockets;
	std::map< int, Client * > _clients;
	PipelineRouteBuilder _builder;
	EventManager _eventManager;

	void setupListenSockets();
	void setupSignalPipe();
	void handleNewConnection(int listenFd);
	void handleSigchldEvent();

	std::string getSessionId(const PipelineContext *ctx) const;
};
