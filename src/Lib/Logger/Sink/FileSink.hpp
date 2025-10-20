#pragma once

#include "BufferingSink.hpp"
#include <fstream>
#include <string>

class FileSink : public BufferingSink {
public:
	FileSink(const std::string &logDir, const std::string &filename,
			 LogForm *form, size_t maxFileSize, size_t maxBackupFiles);
	virtual ~FileSink();

private:
	virtual void flushBuffer();
	void rotate();

	std::string _dir;
	std::string _fileName;
	std::ofstream _fileStream;
	size_t _maxFileSize;
	size_t _maxBackupFiles;
};
