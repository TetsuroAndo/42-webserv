#include "LogSink.hpp"
#include "../Form/LogForm.hpp"

LogSink::LogSink(LogForm *form)
	: _type(errorLog), _form(form) {}

LogSink::LogSink(LogForm *form) 
	: _type(accessLog), _form(form) {}

LogSink::~LogSink() {
	if (_form) {
		delete _form;
		_form = NULL;
	}
}

LogType LogSink::getType() const { return _type; }
LogForm *LogSink::getForm() const { return _form; }
