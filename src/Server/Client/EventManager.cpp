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

void EventManager::initFd(const int fd) { _clientTable[fd] = NULL; }

void EventManager::handle(const int fd, const unsigned int events) {
	if (_eventsTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non-existing fd " << fd;
		return;
	}
	unsigned int effective = events;
	if (events & EPOLLHUP) {
		effective |= EPOLLIN;
	}
	for (std::vector< AEvent * >::const_iterator it = _eventsTable[fd].begin();
		 it != _eventsTable[fd].end(); ++it) {
		if ((*it) == NULL) {
			continue;
		}
		if ((*it)->isExpectedEventType(events) == false) {
			continue;
		}
		(*it)->handle();
		return;
	}
	if (events & EPOLLERR || events & EPOLLHUP) {
		LOG(WARNING) << "EPOLLERR or EPOLLHUP for client fd: " << fd;
		removeFd(fd);
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
	if (_clientTable.count(fd) > 0) {
		LOG(DEBUG) << "adding event fd " << fd;
		_eventsTable[fd].push_back(event);
	}
}

void EventManager::removeFd(const int fd) {
	clearEvents(fd);

	if (_clientTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non-existing fd " << fd;
		return;
	}
	if (_clientTable[fd] != NULL)
		_clientTable[fd]->getServer().closeConnection(fd);
	_clientTable.erase(fd);
}

void EventManager::clearEvents(const int fd) {
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
}
