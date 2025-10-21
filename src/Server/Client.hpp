#pragma once

#include "../Config/Config.hpp"
#include "../Socket/Socket.hpp"
#include <string>

class PipelineContext;

class Client {
public:
	Client(int fd, const sockaddr_in &addr, const Config &config,
		   int serverPort);
	~Client();

	int getFd() const;
	const std::string &getIp() const;
	int getPort() const;
	int getServerPort() const;
	Socket *getSocket() const;
	PipelineContext *getContext() const;

private:
	int _fd;
	std::string _ip;
	int _port;
	int _serverPort;
	Socket *_socket;
	PipelineContext *_context;

	Client(const Client &);
	Client &operator=(const Client &);
};
