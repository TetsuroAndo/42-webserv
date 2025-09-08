#include "SocketsManager.hpp"

#include <stdexcept>
#include <unistd.h>

SocketsManager::SocketsManager() : events(MAX_EVENTS) {
	epoll_fd = epoll_create1(0);
	if (epoll_fd < 0) {
		throw std::runtime_error("epoll_create1() failed");
	}
}

SocketsManager::~SocketsManager() {
	if (epoll_fd >= 0) {
		close(epoll_fd);
	}
}

void SocketsManager::registerSocket(int fd, uint32_t events) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
		throw std::runtime_error("epoll_ctl(ADD) failed");
	}
}

void SocketsManager::modifySocket(int fd, uint32_t events) {
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
		throw std::runtime_error("epoll_ctl(MOD) failed");
	}
}

void SocketsManager::unregisterSocket(int fd) {
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

int SocketsManager::wait(int timeout) {
	int eventSize = epoll_wait(epoll_fd, events.data(), events.size(), timeout);
	if (eventSize < 0) {
		throw std::runtime_error("epoll_wait() failed");
	}
	return eventSize;
}

struct epoll_event *SocketsManager::getEvents() { return events.data(); }
