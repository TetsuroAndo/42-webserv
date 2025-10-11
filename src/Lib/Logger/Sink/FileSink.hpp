#pragma once

#include "LogSink.hpp"
#include <fstream>
#include <string>

class FileSink : public LogSink {
public:
	FileSink(const std::string &logDir, const std::string &filename,
			 LogForm *form, size_t maxFileSize, size_t maxBackupFiles);
	virtual ~FileSink();

	virtual void log(const LogMessage &msg);
	virtual void logAccess(const AccessLogContext& ctx);
private:
	void rotate();
	std::string _dir;
	std::string _fileName;
	std::ofstream _fileStream;
	size_t _maxFileSize;
	size_t _maxBackupFiles;
};
