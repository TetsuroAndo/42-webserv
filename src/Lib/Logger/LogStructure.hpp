#pragma once

#include "../../Http/Core/HttpRequest.hpp"
#include "../../Http/Core/HttpResponse.hpp"
#include <map>
#include <string>

enum LogFormat { JSON, ELF };

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

struct AccessLogContext {
	time_t timestamp;
	const HttpRequest* request;
	const HttpResponse* response;
	std::string remote_addr;
};
