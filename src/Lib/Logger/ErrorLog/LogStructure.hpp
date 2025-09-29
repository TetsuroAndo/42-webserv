#pragma once

#include <ctime>
#include <map>
#include <string>

enum LogFilterMode {
	GREATER_OR_EQUAL, // 指定レベル以上（デフォルト）
	EXACT			  // 指定レベルと完全一致
};

enum LogLevel { DEBUG, INFO, WARNING, ERROR, FATAL };

struct LogMessage {
	time_t timestamp;
	LogLevel level;
	std::string message;
	const char *file;
	int line;
	const char *function;
	std::map<std::string, std::string> attributes;
};
