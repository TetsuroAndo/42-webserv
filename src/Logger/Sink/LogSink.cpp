#include "LogSink.hpp"
#include "../Form/LogForm.hpp"

LogSink::LogSink(LogForm* Form, LogLevel level) : _Form(Form), _logLevel(level) {}

LogSink::~LogSink() {
	delete _Form;
}

LogForm* LogSink::getForm() const {
	return _Form;
}

LogLevel LogSink::getLogLevel() const {
	return _logLevel;
}

void LogSink::setLogLevel(const LogLevel level) {
	_logLevel = level;
}
