#pragma once

#include "ListenKey.hpp"
#include <netinet/in.h>
#include <vector>

class Listener {
public:
	Listener();
	explicit Listener(const ListenKey &key);

	const ListenKey &getKey() const;
	int getFd() const;
	const sockaddr_in &getAddr() const;
	const std::vector< size_t > &getVhostIndices() const;
	size_t getDefaultVhostIndex() const;

	void setSocket(int fd, const sockaddr_in &addr);
	void addVhostIndex(size_t index);

private:
	ListenKey _key;
	int _fd;
	sockaddr_in _addr;
	std::vector< size_t > _vhostIndices;
	size_t _defaultVhostIndex;
};
