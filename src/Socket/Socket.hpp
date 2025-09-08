#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

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
	void appendRecvBuffer(const char* data, int size);
	void eraseSendBuffer(int start, int end);
	const std::string &getRecvBuffer() const;
	const std::string &getSendBuffer() const;

private:
	int fd;
	bool listen;
	sockaddr_in addr;
	std::string recvBuffer;
	std::string sendBuffer;
};

#endif
