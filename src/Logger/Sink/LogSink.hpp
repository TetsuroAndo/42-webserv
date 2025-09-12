#pragma once

#include "../LogStructure.hpp"
#include <string>

class LogForm;

class LogSink {
public:
	LogSink(LogForm *form, LogLevel level = INFO, LogFilterMode mode = GREATER_OR_EQUAL);
	virtual ~LogSink();
	virtual void log(const LogMessage& msg) = 0;

	LogForm* getForm() const;
	LogLevel getLogLevel() const;
	LogFilterMode getFilterMode() const;
	void setLogLevel(const LogLevel level);

protected:
	LogForm* _form;
	LogLevel _logLevel;
	LogFilterMode _filterMode;
};
