#pragma once

#include "../LogStructure.hpp"
#include <string>

class LogForm;

class LogSink {
public:
	LogSink(LogForm* Form, LogLevel level = INFO);
	virtual ~LogSink();
	virtual void write(const std::string& formattedMessage) = 0;
	LogForm* getForm() const;
	LogLevel getLogLevel() const;

	void setLogLevel(const LogLevel level);

protected:
	LogForm* _Form;
	LogLevel _logLevel;
};
