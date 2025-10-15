#pragma once

#include "../AccessLog/AccessLogStructure.hpp"
#include "../ErrorLog/LogStructure.hpp"
#include "../LogType.hpp"

class LogForm;

class LogSink {
public:
	LogSink(LogForm *form);
	virtual ~LogSink();

	virtual void log(const LogMessage &msg) = 0;
	virtual void logAccess(const AccessLogContext &ctx) = 0;

	LogType getType() const;
	LogForm *getForm() const;

protected:
	LogType _type;
	LogForm *_form;
};
