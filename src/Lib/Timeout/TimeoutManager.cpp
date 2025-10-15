#include "TimeoutManager.hpp"
#include "ITimeoutable.hpp"
#include <ctime>
#include <limits>
#include <map>
#include <vector>

TimeoutManager::TimeoutManager() {}

TimeoutManager::~TimeoutManager() {}

void TimeoutManager::add(ITimeoutable *obj, time_t timeoutSec) {
	if (!obj)
		return;

	remove(obj);

	time_t expiryTime = std::time(NULL) + timeoutSec;
	_timeoutMap[expiryTime] = obj;
	_reverseMap[obj] = expiryTime;
}

void TimeoutManager::remove(ITimeoutable *obj) {
	if (!obj)
		return;

	ReverseTimeoutMap::iterator itObj = _reverseMap.find(obj);
	if (itObj != _reverseMap.end()) {
		time_t expiryTime = itObj->second;
		_timeoutMap.erase(expiryTime);
		_reverseMap.erase(itObj);
	}
}

void TimeoutManager::checkAndHandleTimeouts() {
	time_t now = std::time(NULL);
	std::vector< ITimeoutable * > expiredObjects;

	TimeoutMap::iterator it = _timeoutMap.begin();
	while (it != _timeoutMap.end()) {
		if (it->first <= now) {
			expiredObjects.push_back(it->second);
			_reverseMap.erase(it->second);

			TimeoutMap::iterator to_erase = it;
			++it;
			_timeoutMap.erase(to_erase);
		} else {
			// mapはキー（時刻）でソートされているため、これ以上古いものはない
			break;
		}
	}

	for (std::vector< ITimeoutable * >::iterator it = expiredObjects.begin();
		 it != expiredObjects.end(); ++it) {
		(*it)->onTimeout();
	}
}

int TimeoutManager::getNextTimeoutInterval() const {
	if (_timeoutMap.empty()) {
		return -1; // タイムアウトなし
	}

	time_t now = std::time(NULL);
	time_t nextExpiry = _timeoutMap.begin()->first;

	if (nextExpiry <= now) {
		return 0;
	}

	double diff = std::difftime(nextExpiry, now);
	if (diff >
		static_cast< double >(std::numeric_limits< int >::max()) / 1000.0) {
		return std::numeric_limits< int >::max();
	}

	return static_cast< int >(diff * 1000.0);
}
