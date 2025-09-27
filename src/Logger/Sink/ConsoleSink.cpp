#include "../Form/JsonForm.hpp"
#include "ConsoleSink.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(LogForm *form, const LogLevel level, const LogFilterMode mode)
	: LogSink(form, level, mode) {}

void ConsoleSink::log(const LogMessage &msg) {
	_form->format(msg, std::cout);
}
