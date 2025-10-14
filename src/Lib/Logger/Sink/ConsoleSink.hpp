#pragma once

#include "LogSink.hpp"
#include <string>

class ConsoleSink : public LogSink {
public:
	ConsoleSink(LogForm *form);

	virtual void log(const LogMessage &msg);
	virtual void logAccess(const AccessLogContext &ctx);
};
