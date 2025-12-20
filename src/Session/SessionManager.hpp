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
	void cleanupExpiredSessions(const time_t timeoutSec);

private:
	SessionManager();
	SessionManager(const SessionManager &);
	SessionManager &operator=(const SessionManager &);
	~SessionManager();

	std::string generateSessionId() const;

	Token &_hasher;
	std::map< std::string, Session * > _sessions;
};
