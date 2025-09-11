#pragma once

#include "LogSink.hpp"
#include <string>

class ConsoleSink : public LogSink {
public:
	ConsoleSink(LogForm* Form);
	virtual void write(const std::string& formattedMessage);
};
