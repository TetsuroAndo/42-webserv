#include "Logger.hpp"
#include <sstream>
#include <ctime>
#include <stdexcept>

Logger* Logger::_instance = NULL;

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

Logger::Logger() : _logLevel(DEBUG) {}

Logger::~Logger() {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		delete *it;
	}
	_sinks.clear();
}

void Logger::addSink(LogSink* sink) {
	_sinks.push_back(sink);
}

void Logger::setLogLevel(LogLevel level) {
	_logLevel = level;
}

void Logger::log(const LogMessage& msg) {
	if (msg.level < _logLevel) {
		return;
	}

	std::string formatted = formatMessage(msg);
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		(*it)->write(formatted);
	}
}

std::string Logger::levelToString(LogLevel level) const {
	switch (level) {
		case DEBUG: return "DEBUG";
		case INFO: return "INFO";
		case WARNING: return "WARNING";
		case ERROR: return "ERROR";
		case FATAL: return "FATAL";
		default: return "UNKNOWN";
	}
}

std::string Logger::formatMessage(const LogMessage& msg) {
	std::stringstream ss;
	char timeStr[20];
	std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S", localtime(&msg.timestamp));

}
