#include "Session.hpp"
#include <ctime>
#include <string>
#include <map>

Session::Session(const std::string &id) : _sessionId(id), _lastAccessTime(std::time(nullptr)) {}
Session::~Session() {}

const std::string &Session::getId() const {
	return _sessionId;
}

const std::string &Session::getData(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = _data.find(key);
	if (it != _data.end()) {
		return it->second;
	}
	throw std::runtime_error("Data not found");
}

time_t Session::getLastAccess() const {
	return _lastAccessTime;
}

bool Session::hasData(const std::string &key) const {
	return _data.count(key) > 0;
}

void Session::setData(const std::string &key, const std::string &value) {
	_data[key] = value;
}

void Session::updateLastAccess() {
	_lastAccessTime = std::time(NULL);
}
