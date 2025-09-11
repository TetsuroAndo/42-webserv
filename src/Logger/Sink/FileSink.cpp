#include "FileSink.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <dirent.h>
#include <string>
#include <vector>
#include <regex>

namespace {
	size_t getMaxBackupIndex(const std::string& logDir, const std::string& filename) {
		size_t maxIndex = 0;

		std::regex pattern(filename + "\\.(\\d+)$");
		DIR* dir = opendir(logDir.c_str());
		if (!dir) return 0;

		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			std::string fname(entry->d_name);
			std::smatch match;
			if (std::regex_search(fname, match, pattern)) {
				size_t idx = std::strtol(match[1].str().c_str(), nullptr, 10);
				if (idx > maxIndex) maxIndex = idx;
			}
		}
		closedir(dir);
		return maxIndex;
	}

	bool fileRename(const std::string& oldPath, const std::string& newPath) {
		if (std::rename(oldPath.c_str(), newPath.c_str()) != 0) {
			std::cerr << "Logger: Failed to rename log file from " << oldPath << " to " << newPath << std::endl;
			return false;
		}
		return true;
	}

	bool fileRemove(const std::string& path) {
		if (std::remove(path.c_str()) != 0) {
			std::cerr << "Logger: Failed to remove old log file: " << path << std::endl;
			return false;
		}
		return true;
	}
}

FileSink::FileSink(
	const std::string& logDir,
	const std::string &filename,
	LogForm *Form,
	LogLevel level,
	size_t maxFileSize,
	size_t maxBackupFiles
) :
	LogSink(Form, level),
	_file(logDir + "/" + filename, std::ios::out | std::ios::app),
	_dir(logDir),
	_fileName(filename),
	_maxFileSize(maxFileSize),
	_maxBackupFiles(maxBackupFiles)
{

	if (!_file.is_open()) {
		throw std::runtime_error("Logger: Failed to open log file: " + filename);
	}
}

FileSink::~FileSink() {
	if (_file.is_open()) {
		_file.close();
	}
}

void FileSink::write(const std::string &formattedMessage) {
	if (_file.is_open()) {
		_file << formattedMessage << std::endl;
		if (_maxFileSize > 0 &&
			_file.tellp() >= static_cast<std::streampos>(_maxFileSize)
		) {
			_file.close();
			size_t idx = getMaxBackupIndex(_dir, _fileName);
			std::string filepath = _dir + "/" + _fileName;
			if (idx >= _maxBackupFiles) {
				if (!fileRemove(filepath + "." + std::to_string(idx - _maxBackupFiles + 1))) {
					// TODO: エラー処理をどうするか
				}
			}
			if (!fileRename(filepath, filepath + "." + std::to_string(idx + 1))) {
				// TODO: エラー処理をどうするか
			}
			_file.open(filepath, std::ios::out | std::ios::app);
		}
	}
}
