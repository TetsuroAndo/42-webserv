#include "LogSink.hpp"
#include "../Form/LogForm.hpp"

LogSink::LogSink(LogForm *form, const LogLevel level, const LogFilterMode mode)
	: _type(errorLog), _form(form), _logLevel(level), _filterMode(mode) {}

LogSink::LogSink(LogForm *form) 
	: _type(accessLog), _form(form), _logLevel(INFO), _filterMode(GREATER_OR_EQUAL) {}

LogSink::~LogSink() {
	if (_form) {
		delete _form;
		_form = NULL;
	}
}

LogType LogSink::getType() const { return _type; }
LogForm *LogSink::getForm() const { return _form; }
LogLevel LogSink::getLogLevel() const { return _logLevel; }
LogFilterMode LogSink::getFilterMode() const { return _filterMode; }
