#include "ConsoleSink.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/ElfForm.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(const std::string &Form, LogLevel level) : LogSink(level) {
	if (Form == "JSON") {
		LogSink::_Form = new JsonForm();
	} else {
		if (Form != "ELF") {
			std::cerr << "[ WARNING ] Logger: ConsoleSink: Unknown log format: " + Form << std::endl;
		}
		LogSink::_Form = new ElfForm();
	}
}

void ConsoleSink::write(const std::string& formattedMessage) {
	std::cout << formattedMessage << std::endl;
}
