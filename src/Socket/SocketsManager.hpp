#pragma once

#include <sys/epoll.h>
#include <vector>

class SocketsManager {
public:
	SocketsManager();
	~SocketsManager();

	void registerSocket(int fd, uint32_t events) const;
	void modifySocket(int fd, uint32_t events) const;
	void unregisterSocket(int fd) const;
	int wait(int timeout);
	struct epoll_event *getEvents();

private:
	int _epoll_fd;
	std::vector< struct epoll_event > _events;
};
