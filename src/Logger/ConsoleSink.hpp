#pragma once

#include "LogSink.hpp"
#include <string>

class ConsoleSink : public LogSink {
public:
	virtual void write(const std::string& formattedMessage);
};
