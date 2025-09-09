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
	void cleanupExpiredSessions(); // 定期的に呼び出す

private:
	SessionManager();
	SessionManager(const SessionManager&);
	SessionManager& operator=(const SessionManager&);
	~SessionManager();

	Token token;
	std::string generateSessionId();

	std::map<std::string, Session*> _sessions;
	const uint32_t SESSION_TIMEOUT = 1800; // 30分
};
