#pragma once

#include "../Config/Config.hpp"
#include "../Lib/Timeout/ITimeoutable.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "../Socket/Socket.hpp"
#include "HttpConnection.hpp"
#include "HttpConnectionEventHandler.hpp"
#include <netinet/in.h>
#include <string>

class Server; // Serverは参照で持つため前方宣言のままでOK

class Client : public ITimeoutable, public HttpConnectionEventHandler {
public:
	Client(int fd, const sockaddr_in &addr, int listenPort, Server &server);
	~Client();

	/// @brief Serverへのアクセス（HttpConnectionから使用を想定）
	Server &getServer() const;

	const int getFd() const;
	const int getPort() const;
	const int getListenPort() const;
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
	HttpConnection _httpConnection;

	int _fd;
	int _port;
	int _listenPort;
	std::string _ip;
	Socket _socket;
	PipelineContext _context;

	Client(const Client &);
	Client &operator=(const Client &);
};
