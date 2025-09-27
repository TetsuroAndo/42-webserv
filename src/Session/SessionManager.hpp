#pragma once

#include "Session.hpp"
#include "../Lib/Token/Token.hpp"
#include <string>
#include <map>

class SessionManager {
public:
	static SessionManager& getInstance();

	Session* createSession();
	Session* getSession(const std::string& sessionId);
	bool destroySession(const std::string& sessionId);
	void cleanupExpiredSessions(); // TODO: 定期的に呼び出す

private:
	SessionManager();
	SessionManager(const SessionManager&);
	SessionManager& operator=(const SessionManager&);
	~SessionManager();

	Token token;
	std::string generateSessionId() const;

	std::map<std::string, Session*> _sessions;
	static const time_t _SESSION_TIMEOUT = 1800; // 30分
};
