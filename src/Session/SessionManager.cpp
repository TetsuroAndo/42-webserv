#include "SessionManager.hpp"
#include "../Lib/Token/Token.hpp"
#include <ctime>
#include <string>

SessionManager::SessionManager() {}
SessionManager::~SessionManager() {
	for (std::map<std::string, Session*>::iterator it = _sessions.begin(); it != _sessions.end(); ++it) {
		delete it->second;
	}
	_sessions.clear();
}

std::string SessionManager::generateSessionId() {
	std::string sessionId;
	do {
		sessionId = token.genToken(32);
	} while (_sessions.count(sessionId) > 0);
	return sessionId;
}

SessionManager& SessionManager::getInstance() {
	static SessionManager instance;
	return instance;
}

Session* SessionManager::createSession() {
	std::string sessionId = generateSessionId();
	Session* newSession = new Session(sessionId);
	_sessions[sessionId] = newSession;
	return newSession;
}

Session* SessionManager::getSession(const std::string& sessionId) {
	std::map<std::string, Session*>::iterator it = _sessions.find(sessionId);
	if (it != _sessions.end()) {
		it->second->updateLastAccess();
		return it->second;
	}
	return nullptr;
}

bool SessionManager::destroySession(const std::string& sessionId) {
	std::map<std::string, Session*>::iterator it = _sessions.find(sessionId);
	if (it != _sessions.end()) {
		delete it->second;
		_sessions.erase(it);
		return true;
	}
	return false;
	
}

void SessionManager::cleanupExpiredSessions() {
	time_t now = std::time(NULL);
	std::vector<std::string> toDelete;

	std::map<std::string, Session*>::iterator it = _sessions.begin();
	for ( ; it != _sessions.end(); ++it) {
		if (now - it->second->getLastAccess() > SESSION_TIMEOUT) {
			toDelete.push_back(it->first);
		}
	}

	for (size_t i = 0; i < toDelete.size(); ++i) {
		if (!destroySession(toDelete[i])) {
			// セッションの削除に失敗した場合のログ出力やエラーハンドリング
		}
	}
}
