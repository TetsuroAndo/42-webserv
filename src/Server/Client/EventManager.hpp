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
	void handle(int fd, unsigned int events);
	void addEvent(int fd, AEvent *event);
	void removeFd(int fd);
	void clearEvents(int fd);

private:
	std::map< int, std::vector< AEvent * > > _eventsTable;
	std::map< int, Client * > _clientTable;
};
