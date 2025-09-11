#pragma once

#include "LogSink.hpp"
#include <string>
#include <fstream>

class FileSink : public LogSink {
public:
	FileSink(const std::string& filename, const std::string &Form, LogLevel level,
		size_t maxFileSize = _LOG_MAX_FILE_SIZE, size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	FileSink(const std::string& logDir, const std::string& filename,
		const std::string &Form, LogLevel level,
		size_t maxFileSize = _LOG_MAX_FILE_SIZE, size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	virtual ~FileSink();
		
	virtual void write(const std::string& formattedMessage);

private:
	std::string _dir;
	std::string _fileName;
	std::ofstream _fileStream;
	size_t _maxFileSize;
	size_t _maxBackupFiles;
};
