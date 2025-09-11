#include "ConsoleSink.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(LogForm* Form, LogLevel level) : LogSink(Form, level) {}

void ConsoleSink::write(const std::string& formattedMessage) {
	std::cout << formattedMessage << std::endl;
}
