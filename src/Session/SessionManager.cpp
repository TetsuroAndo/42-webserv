#include "SessionManager.hpp"

#include "../Lib/Logger/Log.hpp"

#include <ctime>
#include <string>

SessionManager::SessionManager()
	: _hasher(Token::getInstance()), _timeoutSec(1800),
	  _lastCleanupTime(std::time(NULL)) {}

SessionManager::~SessionManager() {
	for (std::map< std::string, Session * >::iterator it = _sessions.begin();
		 it != _sessions.end(); ++it) {
		delete it->second;
	}
	_sessions.clear();
}

std::string SessionManager::generateSessionId() const {
	std::string sessionId;
	do {
		sessionId = _hasher.genToken();
	} while (_sessions.find(sessionId) != _sessions.end());
	return sessionId;
}

SessionManager &SessionManager::getInstance() {
	static SessionManager instance;
	return instance;
}

Session *SessionManager::createSession() {
	const std::string sessionId = generateSessionId();
	Session *newSession = NULL;
	try {
		newSession = new Session(sessionId);
		_sessions[sessionId] = newSession;
		return newSession;
	} catch (const std::exception &e) {
		delete newSession;
		LOG(ERROR) << "Failed to create Session: " << e.what();
		throw;
	}
}

Session *SessionManager::getSession(const std::string &sessionId) {
	const std::map< std::string, Session * >::iterator it =
		_sessions.find(sessionId);
	if (it != _sessions.end()) {
		const time_t now = std::time(NULL);
		const time_t elapsed = now - it->second->getLastAccess();
		if (_timeoutSec == 0 || elapsed > _timeoutSec) {
			delete it->second;
			_sessions.erase(it);
			return NULL;
		}
		it->second->updateLastAccess();
		return it->second;
	}
	return NULL;
}

bool SessionManager::destroySession(const std::string &sessionId) {
	const std::map< std::string, Session * >::iterator it =
		_sessions.find(sessionId);
	if (it != _sessions.end()) {
		delete it->second;
		_sessions.erase(it);
		return true;
	}
	return false;
}

void SessionManager::cleanupExpiredSessions() {
	const time_t now = std::time(NULL);
	size_t deleteCount = 0;

	std::map< std::string, Session * >::iterator it = _sessions.begin();
	while (it != _sessions.end()) {
		const time_t elapsed = now - it->second->getLastAccess();
		if (_timeoutSec == 0 || elapsed > _timeoutSec) {
			delete it->second;
			_sessions.erase(it++);
			deleteCount++;
		} else {
			++it;
		}
	}
	if (0 < deleteCount) {
		LOG(INFO) << "SessionManager::cleanupExpiredSessions closed Session: "
				  << deleteCount;
	}
}

void SessionManager::cleanupIfNeeded() {
	const time_t now = std::time(NULL);
	// タイムアウト時間が0の場合は毎回クリーンアップ、それ以外はタイムアウト時間経過後にクリーンアップ
	if (_timeoutSec == 0 || now - _lastCleanupTime >= _timeoutSec) {
		cleanupExpiredSessions();
		_lastCleanupTime = now;
	}
}

void SessionManager::setTimeoutSec(time_t timeoutSec) {
	_timeoutSec = timeoutSec;
}
