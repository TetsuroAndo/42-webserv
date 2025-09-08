#ifndef SOCKETSMANAGER_HPP
#define SOCKETSMANAGER_HPP

#include <sys/epoll.h>
#include <vector>

#define MAX_EVENTS 64

class SocketsManager {
public:
	SocketsManager();
	~SocketsManager();

	void registerSocket(int fd, uint32_t events);
	void modifySocket(int fd, uint32_t events);
	void unregisterSocket(int fd);
	int wait(int timeout);
	struct epoll_event *getEvents();

private:
	int epoll_fd;
	std::vector<struct epoll_event> events;
};

#endif
