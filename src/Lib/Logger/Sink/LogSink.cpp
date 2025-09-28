#include "LogSink.hpp"
#include "../Form/LogForm.hpp"

LogSink::LogSink(LogForm *form, const LogLevel level, const LogFilterMode mode)
	: _form(form), _logLevel(level), _filterMode(mode) {}

LogSink::~LogSink() {
	if (_form) {
		delete _form;
		_form = NULL;
	}
}

void LogSink::logAccess(const AccessLogContext& ctx) {
	if (_form) {
		_form->formatAccess(ctx, getStream()); // getStream()は派生クラスで実装
	}
}

LogType LogSink::getType() const { return _type; }

LogForm *LogSink::getForm() const { return _form; }

LogLevel LogSink::getLogLevel() const { return _logLevel; }

LogFilterMode LogSink::getFilterMode() const { return _filterMode; }

void LogSink::setLogLevel(const LogLevel level) { _logLevel = level; }
