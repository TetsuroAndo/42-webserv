#include "../Config.hpp"
#include <iostream>

int main(const int argc, char **argv) {
	(void)argc;
	(void)argv;
	const Config config;

	std::cout << config << std::endl;
	return 0;
}
