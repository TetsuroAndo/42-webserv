#include "LogSink.hpp"
#include "../Form/LogForm.hpp"

LogSink::LogSink(LogLevel level, LogFilterMode mode) : _logLevel(level), _filterMode(mode) {}

LogSink::~LogSink() {
	delete _Form;
}

LogForm* LogSink::getForm() const {
	return _Form;
}

LogLevel LogSink::getLogLevel() const {
	return _logLevel;
}

LogFilterMode LogSink::getFilterMode() const {
	return _filterMode;
}

void LogSink::setLogLevel(const LogLevel level) {
	_logLevel = level;
}
