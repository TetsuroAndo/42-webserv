#include "LogSink.hpp"
#include "../Form/LogForm.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/ElfForm.hpp"

LogSink::LogSink(LogFormat form, LogLevel level, LogFilterMode mode)
	: _logLevel(level), _filterMode(mode)
{
	if (form == JSON) {
		_Form = new JsonForm();
	} else {
		_Form = new ElfForm();
	}
}

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
