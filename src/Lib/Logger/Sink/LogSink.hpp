#pragma once

#include "../LogStructure.hpp"
#include <string>

enum LogType {
	GENERAL_LOG,
	ACCESS_LOG
};

class LogForm;

class LogSink {
public:
	LogSink(LogForm *form, LogLevel level = INFO,
			LogFilterMode mode = GREATER_OR_EQUAL);
	virtual ~LogSink();
	virtual void log(const LogMessage &msg) = 0;
	virtual void logAccess(const AccessLogContext& ctx);

	LogType getType() const;
	LogForm *getForm() const;
	LogLevel getLogLevel() const;
	LogFilterMode getFilterMode() const;
	void setLogLevel(const LogLevel level);

protected:
	LogType _type;
	LogForm *_form;
	LogLevel _logLevel;
	LogFilterMode _filterMode;
	virtual std::ostream& getStream() = 0;
};
