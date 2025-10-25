#pragma once

#include "../Cgi/CgiManager.hpp"
#include "../Config/Config.hpp"
#include "../Lib/Timeout/TimeoutManager.hpp"
#include "../Middleware/Core/MiddlewareProcessor.hpp"
#include "../Middleware/Builder/PipelineRouteBuilder.hpp"
#include "../Socket/FdEventChanges.hpp"
#include "../Socket/Socket.hpp"
#include "../Socket/SocketsManager.hpp"
#include "Client.hpp"
#include <map>

class Server {
public:
	Server(const Config &config);
	~Server();

	void run();
	void closeConnection(int clientFd);
	void applyCgiChanges();

	// Clientから参照されるゲッター
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
	std::map< int, Socket * > _listenSockets;
	std::map< int, Client * > _clients;
	PipelineRouteBuilder _builder;
	MiddlewareProcessor _mainProcessor;

	void setupListenSockets();
	void handleNewConnection(int listenFd);
	// Clientへ移譲
	// void handleClientRead(int clientFd);
	// void handleClientWrite(int clientFd);

	std::string getSessionId(const PipelineContext *ctx) const;
};
