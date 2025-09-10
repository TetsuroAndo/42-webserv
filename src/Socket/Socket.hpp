#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

#include "../Http/HttpRequest.hpp"

class Socket {
public:
	Socket(int fd);
	Socket(int fd, const sockaddr_in &addr);
	~Socket();

	int getFd() const;
	bool isListen() const;
	void setListen();
	const sockaddr_in &getAddr() const;
	void setRecvBuffer(const std::string &str);
	void setSendBuffer(const std::string &str);
	void appendRecvBuffer(const char *data, int size);
	void eraseSendBuffer(int start, int end);
	std::string &getRecvBuffer();
	const std::string &getSendBuffer() const;
	HttpRequest *getRequest();

private:
	int _fd;
	bool _listen;
	sockaddr_in _addr;
	std::string _recvBuffer;
	std::string _sendBuffer;
	HttpRequest *_request;
};

#endif
