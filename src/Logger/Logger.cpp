#include "Logger.hpp"
#include <ctime>
#include <sstream>
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
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		LogSink* sink = *it;
		std::string formatted = sink->getForm()->format(msg);
		sink->write(formatted);
	}
}
