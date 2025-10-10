#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include "../Timeout/ITimeoutable.hpp"
#include "../Socket/Socket.hpp"
#include "server.hpp"
#include <string>

class Client : public ITimeoutable {
public:
	Client(int fd, const sockaddr_in &addr, const Config &config, Server *server);
	~Client();

	int getFd() const;
	const std::string &getIp() const;
	int getPort() const;
	Socket *getSocket() const;
	PipelineContext *getContext() const;

	virtual void onTimeout() override;

private:
	int _fd;
	std::string _ip;
	int _port;
	Socket *_socket;
	PipelineContext *_context;
	Server* _server;

	// Disable copy
	Client(const Client &);
	Client &operator=(const Client &);
};
