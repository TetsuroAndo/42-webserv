#include "../MyYAML.hpp"
#include <iostream>

int main() {
	try {
		MyYAML data("test.yaml");
		std::cout << "show all keys: " << std::endl;
		data.debugAllKeyAndValue();

		const std::vector<std::string> key4 = data.getValue("key4");
		std::cout << std::endl << "show key4: " << std::endl;
		for (int i = 0; i < key4.size(); i++) {
			std::cout << "key4 "<< i << ": " <<  key4[i] << std::endl;
		}
		std::cout << std::endl << "key4 Size:" <<data.getSize("key4") << std::endl;
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}