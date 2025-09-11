#include "Logger.hpp"
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <cerrno>
#include <iostream>

Logger* Logger::_instance = NULL;

Logger& Logger::getInstance() {
	if (_instance == NULL) {
		_instance = new Logger(_DEFAULT_LOG_DIR);
	}
	return *_instance;
}

Logger& Logger::getInstance(const std::string& logDir) {
	if (_instance == NULL) {
		_instance = new Logger(logDir);
	}
	return *_instance;
}

void Logger::cleanup() {
	delete _instance;
	_instance = NULL;
}

Logger::Logger(const std::string& logDir) : _logDir(logDir) {
	if (mkdir(_logDir.c_str(), 0755) != 0) {
		if (errno != EEXIST) {
			throw std::runtime_error("Logger: Failed to create log directory: " + _logDir);
		}
	}
}

Logger::~Logger() {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		delete *it;
	}
	_sinks.clear();
}

void Logger::addSink(LogSink* sink) {
	_sinks.push_back(sink);
}

void Logger::addFileSink(const std::string &filename, const std::string &Form, LogLevel level,
	size_t maxFileSize, size_t maxBackupFiles) {
	_sinks.push_back(new FileSink(_logDir, filename, Form, level, maxFileSize, maxBackupFiles));
}

void Logger::addFileSink(const std::string &logDir, const std::string &filename,
	const std::string &Form, LogLevel level, size_t maxFileSize, size_t maxBackupFiles) {
	_sinks.push_back(new FileSink(logDir, filename, Form, level, maxFileSize, maxBackupFiles));
}

void Logger::addConsoleSink(const std::string &Form, LogLevel level) {
	_sinks.push_back(new ConsoleSink(Form, level));
}

void Logger::log(const LogMessage& msg) {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		LogSink* sink = *it;
		if (msg.level < sink->getLogLevel()) continue;
		std::string formatted = sink->getForm()->format(msg);
		try {
			sink->write(formatted);
		} catch (const std::exception& e) {
			std::cerr << "Logger: Failed to write log: " << e.what() << std::endl;
		}
	}
}
