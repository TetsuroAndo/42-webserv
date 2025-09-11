#pragma once

#include "LogSink.hpp"
#include <string>
#include <fstream>

class FileSink : public LogSink {
public:
	FileSink(const std::string& logDir, const std::string& filename,
		LogFormat form, LogLevel level, LogFilterMode mode,
		size_t maxFileSize, size_t maxBackupFiles);
	virtual ~FileSink();

	virtual void write(const std::string& formattedMessage);

private:
	std::string _dir;
	std::string _fileName;
	std::ofstream _fileStream;
	size_t _maxFileSize;
	size_t _maxBackupFiles;
};
