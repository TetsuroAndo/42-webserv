#pragma once

#include <string>
#include <map>

enum LogFormat {
	JSON,
	ELF
};

enum LogLevel {
	DEBUG,
	INFO,
	WARNING,
	ERROR,
	FATAL
};

struct LogMessage {
	time_t timestamp;
	LogLevel level;
	std::string message;
	const char* file;
	int line;
	const char* function;
	std::map<std::string, std::string> attributes;
};
