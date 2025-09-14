#include "ConsoleSink.hpp"
#include "../Form/ElfForm.hpp"
#include "../Form/JsonForm.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(LogForm *form, LogLevel level, LogFilterMode mode)
	: LogSink(form, level, mode) {}

void ConsoleSink::log(const LogMessage &msg) {
	_form->format(msg, std::cout);
}
