#pragma once

#include <string>

class LogSink {
public:
	virtual ~LogSink() {}
	virtual void write(const std::string& formattedMessage) = 0;
};
