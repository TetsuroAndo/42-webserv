#include "../Form/LogForm.hpp"
#include "LogSink.hpp"

LogSink::LogSink(LogForm *form, const LogLevel level, const LogFilterMode mode)
	: _form(form), _logLevel(level), _filterMode(mode) {}

LogSink::~LogSink() {
	if (_form) {
		delete _form;
		_form = NULL;
	}
}

LogForm* LogSink::getForm() const {
	return _form;
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
