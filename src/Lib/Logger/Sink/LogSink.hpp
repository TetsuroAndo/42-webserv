#pragma once

#include "../LogType.hpp"
#include "../AccessLog/AccessLogStructure.hpp"
#include "../ErrorLog/LogStructure.hpp"

class LogForm;

class LogSink {
public:
	LogSink(LogForm *form, LogLevel level, LogFilterMode mode);
	LogSink(LogForm *form);
	virtual ~LogSink();

	LogType getType() const;
	LogForm *getForm() const;

	LogLevel getLogLevel() const;
	LogFilterMode getFilterMode() const;

protected:
	LogType _type;
	LogForm *_form;

	LogLevel _logLevel;
	LogFilterMode _filterMode;
};
