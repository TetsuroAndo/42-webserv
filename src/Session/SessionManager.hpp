#pragma once

#include "../Lib/Token/Token.hpp"
#include "Session.hpp"
#include <map>
#include <string>

class SessionManager {
public:
	static SessionManager &getInstance();

	Session *createSession();
	Session *getSession(const std::string &sessionId);
	bool destroySession(const std::string &sessionId);
	void cleanupIfNeeded();
	void setTimeoutSec(time_t timeoutSec);

private:
	SessionManager();
	SessionManager(const SessionManager &);
	SessionManager &operator=(const SessionManager &);
	~SessionManager();

	void cleanupExpiredSessions();

	std::string generateSessionId() const;

	Token &_hasher;
	std::map< std::string, Session * > _sessions;
	time_t _timeoutSec;
	time_t _lastCleanupTime;
};
