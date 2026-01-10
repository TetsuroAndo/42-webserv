#include "EventManager.hpp"
#include "../../Lib/Logger/Log.hpp"

EventManager::EventManager(IFdCloser &fdCloser) : _fdCloser(fdCloser) {}

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
	_clientTable[client.getFd()] = true;
}

void EventManager::initFd(const int fd) { _clientTable[fd] = false; }

void EventManager::handle(const int fd, const unsigned int events) {
	if (_eventsTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non-existing fd " << fd;
		return;
	}
	unsigned int effective = events;
	if (events & EPOLLHUP) {
		char buf[8];
		// MSG_PEEKを利用して、データがソケットに届いているかを確認
		const ssize_t recv_res = recv(fd, buf, 1, MSG_PEEK);
		if (0 < recv_res)
			effective |= EPOLLIN;
		else if (recv_res == 0) {
			// MSG_PEEKでrecv()が0を返す場合は、相手側が正常にshutdown/closeしたことを示す。
			// この状態はすでにEPOLLHUPで検知されており、ここでは追加の処理は行わない。
		} else {
			LOG(WARNING) << "recv() returned error code " << recv_res;
		}
	} else if (events & EPOLLERR) {
		LOG(WARNING) << "EPOLLERR for client fd: " << fd;
		removeFd(fd);
	}
	for (std::vector< AEvent * >::const_iterator it = _eventsTable[fd].begin();
		 it != _eventsTable[fd].end(); ++it) {
		if ((*it) == NULL) {
			continue;
		}
		if ((*it)->isExpectedEventType(effective) == false) {
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
	if (_clientTable[fd]) {
		_fdCloser.closeFd(fd);
	}
	_clientTable.erase(fd);
}

void EventManager::clearEvents(const int fd) {
	if (_eventsTable.count(fd) <= 0) {
		LOG(WARNING) << "called on non-existing fd " << fd;
		return;
	}
	for (std::vector< AEvent * >::const_iterator it = _eventsTable[fd].begin();
		 it != _eventsTable[fd].end(); ++it) {
		delete (*it);
	}
	_eventsTable.erase(fd);
}

void EventManager::forgetFd(const int fd) {
	if (_eventsTable.count(fd) > 0) {
		clearEvents(fd);
	}
	_clientTable.erase(fd);
}
