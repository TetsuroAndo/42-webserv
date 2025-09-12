#pragma once
#include "LogStructure.hpp"
#include "Logger.hpp"
#include <sstream>
#include <string>

// A helper struct to hold attribute data for the stream manipulator
struct LogAttribute {
	std::string key;
	std::string value;
};

// This free function will be used as a stream manipulator
template <typename T>
inline LogAttribute attr(const std::string &key, const T &value) {
	std::stringstream ss;
	ss << value;
	LogAttribute attr;
	attr.key = key;
	attr.value = ss.str();
	return attr;
}

class LogBuilder {
public:
	LogBuilder(LogLevel level, const char *file, int line, const char *func);
	~LogBuilder();

	template <typename T> LogBuilder &operator<<(const T &value) {
		_ss << value;
		return *this;
	}

	// Overload operator<< to handle the LogAttribute manipulator
	friend LogBuilder &operator<<(LogBuilder &builder, const LogAttribute &attr) {
		builder._msg.attributes[attr.key] = attr.value;
		return builder;
	}

private:
	LogMessage _msg;
	std::stringstream _ss;
};

#define LOG(level) \
	if (!Logger::getInstance().isLogLevelActive(level)) {} \
	else LogBuilder(level, __FILE__, __LINE__, __func__)
