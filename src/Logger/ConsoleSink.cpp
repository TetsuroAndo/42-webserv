#include "ConsoleSink.hpp"
#include <iostream>

void ConsoleSink::write(const std::string& formattedMessage) {
	std::cout << formattedMessage << std::endl;
}
