#pragma once

#include "Logger.hpp"
#include <sstream>
#include <string>

struct LogAttribute {
	std::string key;
	std::string value;
};

template < typename T >
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

	template < typename T > LogBuilder &operator<<(const T &value) {
		_ss << value;
		return *this;
	}

	friend LogBuilder &operator<<(LogBuilder &builder,
								  const LogAttribute &attr);

private:
	LogMessage _msg;
	std::stringstream _ss;
};

#define LOG(level)                                                             \
	if (Logger::getInstance().isLogLevelActive(level))                         \
	LogBuilder(level, __FILE__, __LINE__, __func__)
