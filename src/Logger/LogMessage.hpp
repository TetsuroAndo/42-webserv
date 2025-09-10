#pragma once

#include <string>
#include <map>
#include <ctime>
#include "LogLevel.hpp"

struct LogMessage {
	time_t timestamp;
	LogLevel level;
	std::string message;
	const char* file;
	int line;
	const char* function;
	std::map<std::string, std::string> attributes;
};
