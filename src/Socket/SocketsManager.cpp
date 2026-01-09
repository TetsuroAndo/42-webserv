#include "SocketsManager.hpp"
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

SocketsManager::SocketsManager(const size_t maxEvents) : _events(maxEvents) {
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

void SocketsManager::registerSocket(const int fd, const uint32_t events) const {
	epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
		const int savedErrno = errno;
		throw std::runtime_error(std::string("epoll_ctl(ADD) failed: ") +
								 strerror(savedErrno));
	}
}

void SocketsManager::modifySocket(const int fd, const uint32_t events) const {
	epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) {
		const int savedErrno = errno;
		throw std::runtime_error(std::string("epoll_ctl(MOD) failed: ") +
								 strerror(savedErrno));
	}
}

void SocketsManager::unregisterSocket(const int fd) const {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

int SocketsManager::wait(const int timeout) {
	while (true) {
		const int eventSize =
			epoll_wait(_epoll_fd, _events.data(), _events.size(), timeout);
		if (eventSize < 0 && errno == EINTR) {
			continue;
		}
		if (eventSize < 0) {
			throw std::runtime_error("epoll_wait() failed");
		}
		return eventSize;
	}
}

epoll_event *SocketsManager::getEvents() { return _events.data(); }
