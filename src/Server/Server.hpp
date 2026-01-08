#pragma once

#include "../Cgi/CgiManager.hpp"
#include "../Config/Config.hpp"
#include "../Lib/Timeout/TimeoutManager.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Socket/SocketsManager.hpp"
#include "Client/Client.hpp"
#include "Client/EventManager.hpp"
#include "Listen/INewConnectionHandler.hpp"
#include "Listen/ListenerSet.hpp"
#include "VHost/VirtualHost.hpp"

#include <map>
#include <string>
#include <vector>

class Server : public INewConnectionHandler {
public:
	Server(const std::vector< Config > &configs);
	~Server();

	void run();
	void closeConnection(int clientFd);

	TimeoutManager &getTimeoutManager();
	SocketsManager &getSocketsManager();
	CgiManager &getCgiManager();

	void handleNewConnection(int listenFd);

private:
	Server();
	Server(const Server &other);
	Server &operator=(const Server &other);

	std::vector< VirtualHost > _vhosts;
	CgiManager _cgiManager;
	TimeoutManager _timeoutManager;
	SocketsManager _socketsManager;
	std::map< int, Client * > _clients;
	PipelineRouteBuilder _builder;
	ListenerSet _listeners;
	std::map< std::string, size_t > _listenHeaderMax;
	EventManager _eventManager;

	std::string getSessionId(const PipelineContext *ctx) const;
};
