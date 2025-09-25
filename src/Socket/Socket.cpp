#include "Socket.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include <unistd.h>

Socket::Socket(int fd) : _fd(fd), _listen(false), _addr() {
}

Socket::Socket(const int fd, const sockaddr_in &addr)
	: _fd(fd), _listen(false), _addr(addr) {
}

Socket::~Socket() {
}

int Socket::getFd() const { return this->_fd; }

bool Socket::isListen() const { return this->_listen; }

void Socket::setListen() { this->_listen = true; }

const sockaddr_in &Socket::getAddr() const { return this->_addr; }

void Socket::setSendBuffer(const std::string &str) { this->_sendBuffer = str; }

void Socket::eraseSendBuffer(int start, int end) {
	this->_sendBuffer.erase(start, end);
}

const std::string &Socket::getSendBuffer() const { return this->_sendBuffer; }
