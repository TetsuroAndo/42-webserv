#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include "../Socket/Socket.hpp"

class Client {
public:
	Client(int fd, const sockaddr_in &addr, const Config &config);
	~Client();

	int getFd() const;
	Socket *getSocket() const;
	PipelineContext *getContext() const;

private:
	int _fd;
	Socket *_socket;
	PipelineContext *_context;

	// Disable copy
	Client(const Client &);
	Client &operator=(const Client &);
};
