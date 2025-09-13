#include "LogSink.hpp"
#include "../Form/LogForm.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/ElfForm.hpp"

LogSink::LogSink(LogForm *form, LogLevel level, LogFilterMode mode)
	: _form(form), _logLevel(level), _filterMode(mode) {}

LogSink::~LogSink() {}

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
