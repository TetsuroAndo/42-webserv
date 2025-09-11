#pragma once

#include "LogSink.hpp"
#include <string>

class ConsoleSink : public LogSink {
public:
	ConsoleSink(const std::string &Form, LogLevel level, LogFilterMode mode);
	virtual void write(const std::string& formattedMessage);
};
