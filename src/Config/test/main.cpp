#include "../Config.hpp"
#include "../ConfigBuilder.hpp"
#include <iostream>

int main(const int argc, char **argv) {
	(void)argc;
	(void)argv;
	const ConfigBuilder builder("test/sample.yaml");
	const Config config = builder.build();

	std::cout << config << std::endl;
	return 0;
}
