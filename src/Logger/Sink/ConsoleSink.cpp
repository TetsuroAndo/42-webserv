#include "ConsoleSink.hpp"
#include "../Form/ElfForm.hpp"
#include "../Form/JsonForm.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(LogFormat form, LogLevel level, LogFilterMode mode)
	: LogSink(form, level, mode) {}

void ConsoleSink::write(const std::string &formattedMessage) {
	std::cout << formattedMessage << std::endl;
}
