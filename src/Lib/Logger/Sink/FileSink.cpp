#include "FileSink.hpp"
#include "../../StringOps/StringOps.hpp"
#include "../Form/ElfForm.hpp"
#include "../Form/JsonForm.hpp"
#include "../Log.hpp"
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

FileSink::FileSink(const std::string &logDir, const std::string &filename,
				   LogForm *form, const size_t maxFileSize,
				   const size_t maxBackupFiles)
	: LogSink(form), _dir(logDir), _fileName(filename),
	  _fileStream((logDir + "/" + filename).c_str(),
				  std::ios::out | std::ios::app),
	  _maxFileSize(maxFileSize), _maxBackupFiles(maxBackupFiles) {
	if (!_fileStream.is_open()) {
		throw std::runtime_error("Logger: Failed to open log file: " +
								 filename);
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
		_fileStream.flush();

		struct stat st;
		const std::string filepath = _dir + "/" + _fileName;
		if (stat(filepath.c_str(), &st) == 0 &&
			static_cast< size_t >(st.st_size) >= _maxFileSize) {
			rotate();
		}
	}
}

void FileSink::logAccess(const AccessLogContext &ctx) {
	if (_fileStream.is_open()) {
		_form->formatAccess(ctx, _fileStream);
		_fileStream << '\n';
		_fileStream.flush();

		struct stat st;
		const std::string filepath = _dir + "/" + _fileName;
		if (stat(filepath.c_str(), &st) == 0 &&
			static_cast< size_t >(st.st_size) >= _maxFileSize) {
			rotate();
		}
	}
}

void FileSink::rotate() {
	_fileStream.flush();
	_fileStream.close();

	const std::string baseFilepath = _dir + "/" + _fileName;
	struct stat st;

	if (_maxBackupFiles == 0) {
		if (stat(baseFilepath.c_str(), &st) == 0) {
			if (std::remove(baseFilepath.c_str()) != 0) {
				LOG(ERROR) << "Failed to remove " << baseFilepath;
				std::cerr << "Error: Failed to remove " << baseFilepath
						  << std::endl;
			}
		}
	} else {
		const std::string oldestBackupPath =
			baseFilepath + "." + StringOps::toString(_maxBackupFiles);
		if (stat(oldestBackupPath.c_str(), &st) == 0) {
			if (std::remove(oldestBackupPath.c_str()) != 0) {
				LOG(ERROR) << "Failed to remove " << oldestBackupPath;
				std::cerr << "Error: Failed to remove " << oldestBackupPath
						  << std::endl;
			}
		}

		for (size_t i = _maxBackupFiles - 1; i > 0; --i) {
			std::string oldPath = baseFilepath + "." + StringOps::toString(i);
			std::string newPath =
				baseFilepath + "." + StringOps::toString(i + 1);
			if (stat(oldPath.c_str(), &st) == 0) {
				if (std::rename(oldPath.c_str(), newPath.c_str()) != 0) {
					LOG(ERROR)
						<< "Failed to rename " << oldPath << " to " << newPath;
					std::cerr << "Error: Failed to rename " << oldPath << " to "
							  << newPath << std::endl;
				}
			}
		}

		if (stat(baseFilepath.c_str(), &st) == 0) {
			if (std::rename(baseFilepath.c_str(),
							(baseFilepath + ".1").c_str()) != 0) {
				LOG(ERROR) << "Failed to rename " << baseFilepath << " to "
						   << (baseFilepath + ".1");
				std::cerr << "Error: Failed to rename " << baseFilepath
						  << " to " << (baseFilepath + ".1") << std::endl;
			}
		}
	}

	_fileStream.open(baseFilepath.c_str(), std::ios::out | std::ios::app);
	if (!_fileStream.is_open()) {
		LOG(ERROR) << "Failed to open log file: " << baseFilepath;
		throw std::runtime_error("Logger: Failed to open log file: " +
								 baseFilepath);
	}
}
