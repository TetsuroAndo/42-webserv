#include "SocketsManager.hpp"
#include "../Lib/Logger/Log.hpp"
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

bool SocketsManager::epollCtlWithRetry(const int op, const int fd,
									   struct epoll_event *event) const {
	while (true) {
		if (epoll_ctl(_epoll_fd, op, fd, event) == 0) {
			return true;
		}
		if (errno == EINTR) {
			continue;
		}
		return false;
	}
}

void SocketsManager::registerSocket(const int fd, const uint32_t events) const {
	epoll_event event;
	event.data.fd = fd;
	event.events = events;

	if (epollCtlWithRetry(EPOLL_CTL_ADD, fd, &event)) {
		return;
	}
	if (errno == EEXIST) {
		if (epollCtlWithRetry(EPOLL_CTL_MOD, fd, &event)) {
			return;
		}
	}
	LOG(ERROR) << "epoll_ctl(ADD) failed: " << strerror(errno);
	throw std::runtime_error("epoll_ctl(ADD) failed");
}

void SocketsManager::modifySocket(const int fd, const uint32_t events) const {
	epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epollCtlWithRetry(EPOLL_CTL_MOD, fd, &event)) {
		return;
	}
	if (errno == ENOENT) {
		if (epollCtlWithRetry(EPOLL_CTL_ADD, fd, &event)) {
			return;
		}
	}
	LOG(ERROR) << "epoll_ctl(MOD) failed: " << strerror(errno);
	throw std::runtime_error("epoll_ctl(MOD) failed");
}

void SocketsManager::unregisterSocket(const int fd) const {
	if (!epollCtlWithRetry(EPOLL_CTL_DEL, fd, NULL)) {
		if (errno != ENOENT) {
			LOG(WARNING) << "epoll_ctl(DEL) failed: " << strerror(errno);
		}
	}
}

int SocketsManager::wait(const int timeout) {
	while (true) {
		const int eventSize =
			epoll_wait(_epoll_fd, _events.data(), _events.size(), timeout);
		if (eventSize < 0 && errno == EINTR) {
			continue;
		}
		if (eventSize < 0) {
			LOG(ERROR) << "epoll_wait() failed: " << strerror(errno);
			throw std::runtime_error("epoll_wait() failed");
		}
		return eventSize;
	}
}

epoll_event *SocketsManager::getEvents() { return _events.data(); }
