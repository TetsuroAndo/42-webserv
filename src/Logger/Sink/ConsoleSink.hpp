#pragma once

#include "LogSink.hpp"
#include <string>

class ConsoleSink : public LogSink {
public:
	ConsoleSink(LogForm* Form, LogLevel level);
	virtual void write(const std::string& formattedMessage);
};
