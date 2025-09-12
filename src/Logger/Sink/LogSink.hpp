#pragma once

#include "../LogStructure.hpp"
#include <string>

class LogForm;

class LogSink {
public:
	LogSink(LogFormat format, LogLevel level = INFO, LogFilterMode mode = GREATER_OR_EQUAL);
	virtual ~LogSink();
	virtual void write(const std::string& formattedMessage) = 0;

	LogForm* getForm() const;
	LogLevel getLogLevel() const;
	LogFilterMode getFilterMode() const;
	void setLogLevel(const LogLevel level);

protected:
	LogForm* _Form;
	LogLevel _logLevel;
	LogFilterMode _filterMode;
};
