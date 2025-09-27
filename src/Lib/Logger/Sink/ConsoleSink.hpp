#pragma once

#include "LogSink.hpp"
#include <string>

class ConsoleSink : public LogSink {
public:
	ConsoleSink(LogForm *form, LogLevel level, LogFilterMode mode);
	virtual void log(const LogMessage &msg);
};
