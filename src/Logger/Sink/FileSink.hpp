#pragma once

#include "LogSink.hpp"
#include <string>
#include <fstream>

class FileSink : public LogSink {
public:
	FileSink(const std::string& filename, LogForm* Form, LogLevel level);
	virtual ~FileSink();
	virtual void write(const std::string& formattedMessage);

private:
	std::ofstream _file;
};
