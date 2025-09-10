#pragma once

#include <string>

class ILogSink {
public:
	virtual ~ILogSink() {}
	virtual void write(const std::string& formattedMessage) = 0;
};
