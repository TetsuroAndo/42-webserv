#include "Listener.hpp"

#include <cstring>

Listener::Listener()
	: _key(), _fd(-1), _addr(), _vhostIndices(), _defaultVhostIndex(0) {
	std::memset(&_addr, 0, sizeof(_addr));
}

Listener::Listener(const ListenKey &key)
	: _key(key), _fd(-1), _addr(), _vhostIndices(), _defaultVhostIndex(0) {
	std::memset(&_addr, 0, sizeof(_addr));
}

const ListenKey &Listener::getKey() const { return _key; }

int Listener::getFd() const { return _fd; }

const sockaddr_in &Listener::getAddr() const { return _addr; }

const std::vector< size_t > &Listener::getVhostIndices() const {
	return _vhostIndices;
}

size_t Listener::getDefaultVhostIndex() const { return _defaultVhostIndex; }

void Listener::setSocket(const int fd, const sockaddr_in &addr) {
	_fd = fd;
	_addr = addr;
}

void Listener::addVhostIndex(const size_t index) {
	if (_vhostIndices.empty()) {
		_defaultVhostIndex = index;
	}
	_vhostIndices.push_back(index);
}
