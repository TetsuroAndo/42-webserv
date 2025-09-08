#include "Socket.hpp"
#include <unistd.h>

Socket::Socket(int fd) : fd(fd), listen(false) {}

Socket::Socket(int fd, const sockaddr_in &addr)
	: fd(fd), listen(false), addr(addr) {}

Socket::~Socket() {}

int Socket::getFd() const { return this->fd; }

bool Socket::isListen() const { return this->listen; }

void Socket::setListen() { this->listen = true; }

const sockaddr_in &Socket::getAddr() const { return this->addr; }

void Socket::setRecvBuffer(const std::string &str) { this->recvBuffer = str; }

void Socket::setSendBuffer(const std::string &str) { this->sendBuffer = str; }

void Socket::appendRecvBuffer(const std::string &str, int size) {
	this->recvBuffer.append(str, size);
}

void Socket::eraseSendBuffer(int start, int end) {
	this->sendBuffer.erase(start, end);
}

const std::string &Socket::getRecvBuffer() { return this->recvBuffer; }

const std::string &Socket::getSendBuffer() { return this->sendBuffer; }
