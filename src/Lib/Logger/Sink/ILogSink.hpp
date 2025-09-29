#pragma once

struct LogMessage;
struct AccessLogContext;

class IErrorLogSink {
public:
	virtual ~IErrorLogSink() {}
	virtual void log(const LogMessage &msg) = 0;
};

class IAccessLogSink {
public:
	virtual ~IAccessLogSink() {}
	virtual void logAccess(const AccessLogContext& ctx) = 0;
};
