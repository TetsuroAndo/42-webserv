#pragma once

#include "LogMessage.hpp"
#include <string>
#include <sstream>

class LogBuilder {
public:
	LogBuilder(LogLevel level, const char* file, int line);
	~LogBuilder();

	template <typename T>
	LogBuilder& operator<<(const T& value) {
		_ss << value;
		return *this;
	}

	LogBuilder& addAttribute(const std::string& key, const std::string& value);

	private:
	LogMessage _msg;
	std::stringstream _ss;
};

#define LOG(level) LogBuilder(level, __FILE__, __LINE__)
