#pragma once

#include "../LogType.hpp"
#include "../AccessLog/AccessLogStructure.hpp"

class LogForm;

class LogSink {
public:
	LogSink(LogForm *form);
	virtual ~LogSink();

	virtual void log(const LogMessage &msg) = 0;
	virtual void logAccess(const AccessLogContext& ctx) = 0;

	LogType getType() const;
	LogForm *getForm() const;

protected:
	LogType _type;
	LogForm *_form;
};
