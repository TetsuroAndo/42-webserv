#pragma once

#include <map>
#include <string>

class Session{
public:
	Session(const std::string &id);
	~Session();

	// Getters
	const std::string &getId() const;
	const std::string &getData(const std::string &key) const;
	const std::string &getOptionalData(const std::string &key, const std::string &defaultValue = "") const;
	time_t getLastAccess() const;
	bool hasData(const std::string &key) const;

	// Setters
	void setData(const std::string &key, const std::string &value);
	void updateLastAccess();

private:
	std::string _sessionId;
	std::map<std::string, std::string> _data;
	time_t _lastAccessTime;

	// Sessions should not be copyable to ensure uniqueness.
	Session(const Session &other);
	Session &operator=(const Session &other);
};
