#pragma once

#include "LogSink.hpp"
#include <string>
#include <fstream>

class FileSink : public LogSink {
public:
	FileSink(const std::string& filename, LogForm* Form, LogLevel level,
		size_t maxFileSize = 10 * 1024 * 1024, size_t maxBackupFiles = 8);
	FileSink(const std::string& logDir, const std::string& filename, LogForm* Form,
		LogLevel level, size_t maxFileSize = 10 * 1024 * 1024, size_t maxBackupFiles = 8);
	virtual ~FileSink();
	virtual void write(const std::string& formattedMessage);

private:
	std::string _dir;
	std::string _fileName;
	std::ofstream _fileStream;
	size_t _maxFileSize;
	size_t _maxBackupFiles;
};
