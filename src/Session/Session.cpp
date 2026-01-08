#include "Session.hpp"
#include <ctime>
#include <map>
#include <stdexcept>
#include <string>

Session::Session(const std::string &id, const time_t timeoutSec)
	: _sessionId(id), _lastAccessTime(std::time(NULL)),
	  _timeoutSec(timeoutSec) {}

Session::~Session() {}

const std::string &Session::getId() const { return _sessionId; }

const std::string &Session::getData(const std::string &key) const {
	const std::map< std::string, std::string >::const_iterator it =
		_data.find(key);
	if (it == _data.end()) {
		throw std::runtime_error("Required session data not found: " + key);
	}
	return it->second;
}

std::string Session::getOptionalData(const std::string &key,
									 const std::string &defaultValue) const {
	const std::map< std::string, std::string >::const_iterator it =
		_data.find(key);
	if (it == _data.end()) {
		return defaultValue;
	}
	return it->second;
}

time_t Session::getLastAccess() const { return _lastAccessTime; }
time_t Session::getTimeoutSec() const { return _timeoutSec; }

bool Session::hasData(const std::string &key) const {
	return _data.find(key) != _data.end();
}

void Session::setData(const std::string &key, const std::string &value) {
	_data[key] = value;
}

void Session::updateLastAccess() { _lastAccessTime = std::time(NULL); }
