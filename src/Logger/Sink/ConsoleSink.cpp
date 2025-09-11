#include "ConsoleSink.hpp"
#include <iostream>

ConsoleSink::ConsoleSink(LogForm* Form) : LogSink(Form) {}

void ConsoleSink::write(const std::string& formattedMessage) {
	std::cout << formattedMessage << std::endl;
}
