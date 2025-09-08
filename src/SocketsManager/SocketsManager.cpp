#include "SocketsManager.hpp"

#include <stdexcept>
#include <unistd.h>

SocketsManager::SocketsManager() : _events(MAX_EVENTS) {
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd < 0) {
		throw std::runtime_error("epoll_create1() failed");
	}
}

SocketsManager::~SocketsManager() {
	if (_epoll_fd >= 0) {
		close(_epoll_fd);
	}
}

void SocketsManager::registerSocket(int fd, uint32_t events) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
		throw std::runtime_error("epoll_ctl(ADD) failed");
	}
}

void SocketsManager::modifySocket(const int fd, const uint32_t events) const {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
		throw std::runtime_error("epoll_ctl(MOD) failed");
	}
}

void SocketsManager::unregisterSocket(const int fd) const {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

int SocketsManager::wait(const int timeout) {
	int eventSize = epoll_wait(_epoll_fd, _events.data(), _events.size(), timeout);
	if (eventSize < 0) {
		throw std::runtime_error("epoll_wait() failed");
	}
	return eventSize;
}

struct epoll_event *SocketsManager::getEvents() { return _events.data(); }
