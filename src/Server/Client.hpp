#pragma once

#include "../Config/Config.hpp"
#include "../Lib/Timeout/ITimeoutable.hpp"
#include "../Socket/Socket.hpp"
#include "HttpConnectionEventHandler.hpp"
#include <netinet/in.h>
#include <string>

struct PipelineContext;
class CgiManager;
class Server;
class HttpConnection;

class Client : public ITimeoutable, public HttpConnectionEventHandler {
public:
	Client(int fd, const sockaddr_in &addr, const int listenPort,
		   CgiManager &cgiManager, const Config &config, Server *server);
	~Client();

	int getFd() const;
	Socket *getSocket() const;
	PipelineContext *getContext() const;
	const std::string &getIp() const;
	int getPort() const;
	int getListenPort() const;

	virtual void onTimeout();

	// (追加) Serverから委譲されるイベント
	void handleReadEvent();
	void handleWriteEvent();
	void updateTimeout();

	// HttpConnectionへのアクセス
	HttpConnection *getHttpConnection() const;

	// Serverへのアクセス（HttpConnectionから使用）
	Server *getServer() const;

	// HttpConnectionEventHandlerの実装
	virtual void onConnectionClose(int fd);
	virtual void onSocketModify(int fd, uint32_t events);
	virtual void onCgiChanges();
	virtual void onRequestProcessed();

private:
	int _fd;
	std::string _ip;
	int _port;
	int _listenPort;
	Socket *_socket;
	PipelineContext *_context;
	HttpConnection *_httpConnection;
	Server *_server;

	Client(const Client &);
	Client &operator=(const Client &);
};
