#include "EventManager.hpp"

#include "../../Lib/Logger/Log.hpp"
#include "../Server.hpp"

EventManager::EventManager() {}

EventManager::~EventManager() {
	for (std::map< int, std::vector< AEvent * > >::const_iterator it =
			 _eventsTable.begin();
		 it != _eventsTable.end(); ++it) {
		for (std::vector< AEvent * >::const_iterator jt = it->second.begin();
			 jt != it->second.end(); ++jt) {
			delete *jt;
		}
	}
}

void EventManager::initFd(Client &client) {
	if (0 < _clientTable.count(client.getFd())) {
		LOG(WARNING) << "client fd " << client.getFd() << " already exists";
	}
	_clientTable[client.getFd()] = &client;
}

void EventManager::handle(const int fd, const unsigned int events) {
	if (_eventsTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non-existing fd " << fd;
		return;
	}
	if (events & EPOLLERR || events & EPOLLHUP) {
		LOG(WARNING) << "EPOLLERR or EPOLLHUP for client fd: " << fd;
		removeEvents(fd);
		return;
	}
	for (std::vector< AEvent * >::const_reverse_iterator it =
			 _eventsTable[fd].rbegin();
		 it != _eventsTable[fd].rend(); ++it) {
		if ((*it) == NULL) {
			continue;
		}
		if ((*it)->isExpectedEventType(events) == false) {
			continue;
		}
		(*it)->handle();
		return;
	}
}

void EventManager::addEvent(const int fd, AEvent *event) {
	if (_clientTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non init fd " << fd;
	}
	if (event == NULL) {
		LOG(WARNING) << "event is null" << fd;
		return;
	}
	event->setFd(fd);
	_eventsTable[fd].push_back(event);
}

void EventManager::removeEvents(const int fd) {
	if (_eventsTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non-existing fd " << fd;
		return;
	}
	for (std::vector< AEvent * >::const_iterator it = _eventsTable[fd].begin();
		 it != _eventsTable[fd].end(); ++it) {
		(*it)->close();
		delete (*it);
	}
	_eventsTable.erase(fd);

	if (_clientTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non-existing fd " << fd;
		return;
	}
	_clientTable[fd]->getServer().closeConnection(fd);
	_clientTable.erase(fd);
}
