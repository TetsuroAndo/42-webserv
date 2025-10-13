#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include "../Socket/Socket.hpp"
#include <string>

class Client {
public:
	Client(int fd, const sockaddr_in &addr, const Config &config);
	~Client();

	int getFd() const;
	const std::string &getIp() const;
	int getPort() const;
	Socket *getSocket() const;
	PipelineContext *getContext() const;

private:
	int _fd;
	std::string _ip;
	int _port;
	Socket *_socket;
	PipelineContext *_context;

	Client(const Client &);
	Client &operator=(const Client &);
};
