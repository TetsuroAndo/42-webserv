#pragma once
#include "Events/AEvent.hpp"
#include "IFdCloser.hpp"
#include <map>
#include <vector>

class Client;
class AEvent;

class EventManager {
public:
	EventManager(IFdCloser &fdCloser);
	~EventManager();

	void initFd(Client &client);
	void initFd(int fd);
	void handle(int fd, unsigned int events);
	void addEvent(int fd, AEvent *event);
	void removeFd(int fd);
	void clearEvents(int fd);
	void forgetFd(int fd);

private:
	std::map< int, std::vector< AEvent * > > _eventsTable;
	std::map< int, bool > _clientTable;
	IFdCloser &_fdCloser;
};
