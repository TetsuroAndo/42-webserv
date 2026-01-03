#pragma once
#include "Events/AEvent.hpp"
#include <map>
#include <vector>

class Client;
class AEvent;

class EventManager {
public:
	EventManager();
	~EventManager();

	void initFd(Client &client);
	void initFd(int fd);
	void handle(int fd, unsigned int events);
	void addEvent(int fd, AEvent *event);
	bool checkCanRemoveFd(int fd, unsigned int events);
	void removeFd(int fd);
	void clearEvents(int fd);

private:
	std::map< int, std::vector< AEvent * > > _eventsTable;
	std::map< int, Client * > _clientTable;
};
