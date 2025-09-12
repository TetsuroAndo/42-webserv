#include "FileSink.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/ElfForm.hpp"
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

namespace {
	template <typename T>
	std::string numberToString(T number) {
		std::stringstream ss;
		ss << number;
		return ss.str();
	}

	bool isNumeric(const char* s) {
		if (s == NULL || *s == '\0') {
			return false;
		}
		while (*s) {
			if (!std::isdigit(*s)) {
				return false;
			}
			s++;
		}
		return true;
	}

	size_t getMaxBackupIndex(const std::string& logDir, const std::string& filename) {
		size_t maxIndex = 0;
		DIR* dir = opendir(logDir.c_str());
		if (!dir) return 0;

		std::string prefix = filename + ".";
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string name(entry->d_name);
			if (name.find(prefix) == 0) {
				const char* suffix = name.c_str() + prefix.length();
				if (isNumeric(suffix)) {
					std::istringstream iss(suffix);
					size_t idx;
					iss >> idx;
					if (idx > 0 && idx > maxIndex) {
						maxIndex = idx;
					}
				}
			}
		}
		closedir(dir);
		return maxIndex;
	}
}

FileSink::FileSink(
	const std::string& logDir,
	const std::string &filename,
	LogForm *form, LogLevel level, LogFilterMode mode,
	size_t maxFileSize,
	size_t maxBackupFiles
) :
	LogSink(form, level, mode),
	_dir(logDir),
	_fileName(filename),
	_fileStream((logDir + "/" + filename).c_str(), std::ios::out | std::ios::app),
	_maxFileSize(maxFileSize),
	_maxBackupFiles(maxBackupFiles)
{
	if (!_fileStream.is_open()) {
		throw std::runtime_error("Logger: Failed to open log file: " + filename);
	}
}

FileSink::~FileSink() {
	if (_fileStream.is_open()) {
		_fileStream.close();
	}
}

void FileSink::log(const LogMessage &msg) {
	if (_fileStream.is_open()) {
		_form->format(msg, _fileStream);
		_fileStream << '\n';
		if (_maxFileSize > 0
			&& static_cast<size_t>(_fileStream.tellp()) >= _maxFileSize)
		{
			_fileStream.close();

			std::string baseFilepath = _dir + "/" + _fileName;
			size_t lastIdx = getMaxBackupIndex(_dir, _fileName);
			for (size_t i = lastIdx; i > 0; --i) {
				std::string oldPath = baseFilepath + "." + numberToString(i);
				std::string newPath = baseFilepath + "." + numberToString(i + 1);
				if (i >= _maxBackupFiles) {
					if (std::remove(oldPath.c_str()) != 0) {
						throw std::runtime_error("Logger: Failed to remove log files: " + oldPath);
					}
				} else {
					if (std::rename(oldPath.c_str(), newPath.c_str()) != 0) {
						throw std::runtime_error("Logger: Failed to rename log files: " + oldPath + " to " + newPath);
					}
				}
			}
			if (std::rename(baseFilepath.c_str(), (baseFilepath + ".1").c_str()) != 0) {
				throw std::runtime_error("Logger: Failed to rename log files: " + baseFilepath + " to " + (baseFilepath + ".1"));
			}
			_fileStream.open(baseFilepath.c_str(), std::ios::out | std::ios::app);
		}
		_fileStream.flush();
	}
}
