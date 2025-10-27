#include "TimeoutManager.hpp"
#include "ITimeoutable.hpp"
#include <algorithm>
#include <ctime>
#include <limits>
#include <list>
#include <map>
#include <vector>

TimeoutManager::TimeoutManager() {}

TimeoutManager::~TimeoutManager() {}

void TimeoutManager::add(ITimeoutable *obj, time_t timeoutSec) {
	if (!obj)
		return;

	remove(obj);

	time_t expiryTime = std::time(NULL) + timeoutSec;

	// 同じタイムアウト時刻のオブジェクトをリストで管理
	if (_timeoutMap.find(expiryTime) == _timeoutMap.end()) {
		_timeoutMap[expiryTime] = std::list< ITimeoutable * >();
	}
	_timeoutMap[expiryTime].push_back(obj);
	_reverseMap[obj] = expiryTime;
	_stats.incrementActiveConnections();
}

void TimeoutManager::remove(ITimeoutable *obj) {
	if (!obj)
		return;

	ReverseTimeoutMap::iterator itObj = _reverseMap.find(obj);
	if (itObj != _reverseMap.end()) {
		time_t expiryTime = itObj->second;

		TimeoutMap::iterator itTime = _timeoutMap.find(expiryTime);
		if (itTime != _timeoutMap.end()) {
			std::list< ITimeoutable * >::iterator listIt =
				itTime->second.begin();
			while (listIt != itTime->second.end()) {
				if (*listIt == obj) {
					listIt = itTime->second.erase(listIt);
					break;
				} else {
					++listIt;
				}
			}
			if (itTime->second.empty()) {
				_timeoutMap.erase(itTime);
			}
		}
		_reverseMap.erase(itObj);
		_stats.decrementActiveConnections();
	}
}

void TimeoutManager::checkAndHandleTimeouts() {
	time_t now = std::time(NULL);
	std::vector< ITimeoutable * > expiredObjects;

	TimeoutMap::iterator it = _timeoutMap.begin();
	while (it != _timeoutMap.end()) {
		if (it->first <= now) {
			// 同じタイムアウト時刻のすべてのオブジェクトを処理
			for (std::list< ITimeoutable * >::iterator listIt =
					 it->second.begin();
				 listIt != it->second.end(); ++listIt) {
				expiredObjects.push_back(*listIt);
				_reverseMap.erase(*listIt);
				_stats.decrementActiveConnections();
			}

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
		_stats.incrementTimeouts();
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

size_t TimeoutManager::getActiveTimeoutCount() const {
	return _stats.getActiveConnections();
}
