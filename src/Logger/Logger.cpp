#include "Logger.hpp"
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>

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
	mkdir(_logDir.c_str(), 0755);
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

void Logger::log(const LogMessage& msg) {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		LogSink* sink = *it;
		if (msg.level < sink->getLogLevel()) continue;
		std::string formatted = sink->getForm()->format(msg);
		sink->write(formatted);
	}
}
