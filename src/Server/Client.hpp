#pragma once

#include "../Config/Config.hpp"
#include "../Socket/Socket.hpp"
#include <string>

struct PipelineContext;

class Client {
public:
	Client(int fd, const sockaddr_in &addr, const int listenPort,
		   const Config &config);
	~Client();

	int getFd() const;
	Socket *getSocket() const;
	PipelineContext *getContext() const;
	const std::string &getIp() const;
	int getPort() const;
	int getListenPort() const;

private:
	int _fd;
	std::string _ip;
	int _port;
	int _listenPort;
	Socket *_socket;
	PipelineContext *_context;

	Client(const Client &);
	Client &operator=(const Client &);
};
