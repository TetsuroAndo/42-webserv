#pragma once

#include "LogSink.hpp"
#include "ILogSink.hpp"
#include <string>

class ConsoleSink : public LogSink, public IErrorLogSink, public IAccessLogSink {
public:
	ConsoleSink(LogForm *form, LogLevel level, LogFilterMode mode);
	ConsoleSink(LogForm *form);
	virtual void log(const LogMessage &msg);
	virtual void logAccess(const AccessLogContext& ctx);
};
