#pragma once

#include "../Config/Config.hpp"
#include "../Lib/Timeout/ITimeoutable.hpp"
#include "../Socket/Socket.hpp"
#include "Server.hpp"
#include <netinet/in.h>
#include <string>

struct PipelineContext;
class CgiManager;

class Client : public ITimeoutable {
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

	virtual void onTimeout() override;

private:
	int _fd;
	std::string _ip;
	int _port;
	int _listenPort;
	Socket *_socket;
	PipelineContext *_context;
	Server *_server;

	Client(const Client &);
	Client &operator=(const Client &);
};
