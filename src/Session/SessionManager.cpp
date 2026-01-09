#include "SessionManager.hpp"

#include "../Lib/Logger/Log.hpp"

#include <ctime>
#include <string>

SessionManager::SessionManager()
	: _hasher(Token::getInstance()), _nextCleanupTime(0) {}

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

Session *SessionManager::createSession(const time_t timeoutSec) {
	const std::string sessionId = generateSessionId();
	Session *newSession = NULL;
	try {
		newSession = new Session(sessionId, timeoutSec);
		const bool wasEmpty = _sessions.empty();
		_sessions[sessionId] = newSession;
		if (timeoutSec == 0) {
			_nextCleanupTime = 0;
		} else {
			const time_t expiryTime =
				newSession->getLastAccess() + timeoutSec;
			if (wasEmpty || _nextCleanupTime == 0 ||
				expiryTime < _nextCleanupTime) {
				_nextCleanupTime = expiryTime;
			}
		}
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
		Session *session = it->second;
		const time_t now = std::time(NULL);
		const time_t elapsed = now - session->getLastAccess();
		if (session->getTimeoutSec() == 0 ||
			elapsed > session->getTimeoutSec()) {
			delete session;
			_sessions.erase(it);
			return NULL;
		}
		session->updateLastAccess();
		return session;
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
	time_t nextExpiryTime = 0;

	std::map< std::string, Session * >::iterator it = _sessions.begin();
	while (it != _sessions.end()) {
		Session *session = it->second;
		const time_t timeoutSec = session->getTimeoutSec();
		const time_t elapsed = now - session->getLastAccess();
		if (timeoutSec == 0 || elapsed > timeoutSec) {
			delete it->second;
			_sessions.erase(it++);
			deleteCount++;
		} else {
			const time_t expiryTime = session->getLastAccess() + timeoutSec;
			if (nextExpiryTime == 0 || expiryTime < nextExpiryTime) {
				nextExpiryTime = expiryTime;
			}
			++it;
		}
	}
	_nextCleanupTime = nextExpiryTime;
	if (0 < deleteCount) {
		LOG(INFO) << "SessionManager::cleanupExpiredSessions closed Session: "
				  << deleteCount;
	}
}

void SessionManager::cleanup() {
	const time_t now = std::time(NULL);
	if (_nextCleanupTime == 0 || _nextCleanupTime <= now) {
		cleanupExpiredSessions();
	}
}
