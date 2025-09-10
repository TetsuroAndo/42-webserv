#pragma once

#include "ILogSink.hpp"
#include <string>

class ConsoleSink : public ILogSink {
public:
	virtual void write(const std::string& formattedMessage);
};
