#pragma once

#include "LogSink.hpp"
#include <string>

class ConsoleSink : public LogSink {
public:
	ConsoleSink(const std::string &Form, LogLevel level);
	virtual void write(const std::string& formattedMessage);
};
