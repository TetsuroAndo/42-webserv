#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "../Config/Config.hpp"
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class Socket {
public:
	Socket(const Config &c, int fd);
	Socket(const Config &c, int fd, const sockaddr_in &addr);
	~Socket();

	int getFd() const;
	bool isListen() const;
	void setListen();
	const sockaddr_in &getAddr() const;
	void setSendBuffer(const std::string &str);
	void eraseSendBuffer(int start, int end);
	const std::string &getSendBuffer() const;

private:
	int _fd;
	bool _listen;
	sockaddr_in _addr;
	std::string _sendBuffer;
};

#endif
