#include "../MyYAML.hpp"
#include <iostream>

int main() {
	try {
		MyYAML data("test.yaml");
		data.debugAllKeyAndValue();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}