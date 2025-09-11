#pragma once

#include <string>

class LogForm;

class LogSink {
public:
	LogSink(LogForm* Form);
	virtual ~LogSink();
	virtual void write(const std::string& formattedMessage) = 0;
	LogForm* getForm();

protected:
	LogForm* _Form;
};
