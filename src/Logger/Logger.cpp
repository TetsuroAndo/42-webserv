#include "Logger.hpp"
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <iostream>

Logger* Logger::_instance = NULL;
std::string Logger::_logDir = _LOG_DEFAULT_DIR;

void Logger::setLogDir(const std::string &logDir) {
	if (_instance) {
		throw std::logic_error("Logger::setLogDir() must be called before the first call to Logger::getInstance().");
	}
	struct stat st;
	if (stat(logDir.c_str(), &st) != 0) {
		std::cerr << "[ WARNING ] Logger: Log directory does not exist: " + logDir
				  << ", using default directory: " << _logDir << std::endl;
	} else if (!S_ISDIR(st.st_mode)) {
		std::cerr << "[ WARNING ] Logger: Log path exists but is not a directory: " + logDir
				  << ", using default directory: " << _logDir << std::endl;
	} else {
		_logDir = logDir;
	}
}

Logger& Logger::getInstance() {
	if (_instance == NULL) {
		_instance = new Logger();
	}
	return *_instance;
}

void Logger::cleanup() {
	delete _instance;
	_instance = NULL;
}

Logger::Logger() {
	struct stat st;
	if (stat(_logDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
		_logDir = _LOG_FALLBACK_DIR;
		std::cerr << "[ ERROR ] Logger: Default log directory does not exist,"
				  << " using fallback directory: " << _logDir << std::endl;
	}
}

Logger::~Logger() {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		delete *it;
	}
	_sinks.clear();
}

void Logger::setSinkFile(const std::string &filename, LogFormat format,
	LogLevel level, LogFilterMode mode, size_t maxFileSize, size_t maxBackupFiles) {
	_sinks.push_back(new FileSink(_logDir, filename, format, level, mode, maxFileSize, maxBackupFiles));
}

void Logger::setSinkFile(const std::string &logDir, const std::string &filename,
	LogFormat format, LogLevel level, LogFilterMode mode,
	size_t maxFileSize, size_t maxBackupFiles) {
	_sinks.push_back(new FileSink(logDir, filename, format, level, mode, maxFileSize, maxBackupFiles));
}

void Logger::setSinkConsole(LogFormat format, LogLevel level, LogFilterMode mode) {
	_sinks.push_back(new ConsoleSink(format, level, mode));
}

void Logger::log(const LogMessage& msg) {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		LogSink* sink = *it;

		if ((sink->getFilterMode() == EXACT && msg.level != sink->getLogLevel()) ||
			(sink->getFilterMode() == GREATER_OR_EQUAL && msg.level < sink->getLogLevel()))
		{
			continue;
		}

		std::string formatted = sink->getForm()->format(msg);
		try {
			sink->write(formatted);
		} catch (const std::exception& e) {
			std::cerr << "[ ERROR ] Logger: Failed to write log: " << e.what() << std::endl;
		}
	}
}
