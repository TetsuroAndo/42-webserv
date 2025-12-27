#pragma once

#include "../../Config/Config.hpp"
#include "../../Lib/Timeout/ITimeoutable.hpp"
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
	Client(int fd, const sockaddr_in &addr, int listenPort, Server &server,
		   EventManager &eventManager);
	~Client();

	/// @brief Serverへのアクセス（HttpConnectionから使用を想定）
	Server &getServer() const;

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

	/// @brief Timeout処理
	virtual void onTimeout();

	// Serverから委譲されるイベント
	// TODO: ここをEventで一本化する
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
