#pragma once

#include "../../Config/Config.hpp"
#include "../../Lib/Timeout/ITimeoutable.hpp"
#include "../../Middleware/Core/MiddlewareProcessor.hpp"
#include "../../Middleware/Core/PipelineContext.hpp"
#include "../../Socket/Socket.hpp"
#include "EventManager.hpp"
#include "HttpConnection.hpp"
#include "HttpConnectionEventHandler.hpp"
#include <netinet/in.h>
#include <string>

class Server;
class EventManager;

class Client : public ITimeoutable, public HttpConnectionEventHandler {
public:
	Client(int fd, const sockaddr_in &addr, int listenPort,
		   const Config &config, MiddlewareProcessor &mainProcessor,
		   Server &server, EventManager &eventManager);
	~Client();

	/// @brief Serverへのアクセス（HttpConnectionから使用を想定）
	Server &getServer() const;
	const Config &getConfig() const;
	MiddlewareProcessor &getMainProcessor();

	int getFd() const;
	int getPort() const;
	int getListenPort() const;
	const std::string &getIp() const;

	Socket &getSocket();
	const Socket &getSocket() const;
	PipelineContext &getContext();
	const PipelineContext &getContext() const;

	HttpConnection &getHttpConnection();
	const HttpConnection &getHttpConnection() const;

	EventManager &getEventManager() const;

	/// @brief Timeout処理
	virtual void onTimeout();

	void handleReadEvent();
	void handleWriteEvent();
	void updateTimeout();

	// HttpConnectionEventHandlerの実装
	virtual void onConnectionClose(int fd);
	virtual void onSocketModify(int fd, uint32_t events);
	virtual void onCgiChanges();
	virtual void onRequestProcessed();

private:
	Server &_server;
	const Config &_config;
	MiddlewareProcessor &_mainProcessor;
	int _fd;
	int _port;
	int _listenPort;
	std::string _ip;
	Socket _socket;
	PipelineContext _context;
	HttpConnection _httpConnection;
	EventManager &_eventManager;

	Client(const Client &);
	Client &operator=(const Client &);
};
