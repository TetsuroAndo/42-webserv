#include "LogSink.hpp"
#include "../Form/LogForm.hpp"

LogSink::LogSink(LogForm* Form) : _Form(Form) {}

LogSink::~LogSink() {
	delete _Form;
}

LogForm* LogSink::getForm() {
	return _Form;
}
