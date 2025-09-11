#include "ConsoleSink.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/ElfForm.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(LogFormat form, LogLevel level, LogFilterMode mode) : LogSink(level, mode) {
	if (form == JSON) {
		LogSink::_Form = new JsonForm();
	} else {
		LogSink::_Form = new ElfForm();
	}
}

void ConsoleSink::write(const std::string& formattedMessage) {
	std::cout << formattedMessage << std::endl;
}
