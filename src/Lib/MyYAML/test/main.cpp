#include "../MyYAML.hpp"
#include <iostream>

int main() {
	try {
		MyYAML data("test.yaml");
		std::cout << "show all keys: " << "\n";
		data.debugAllKeyAndValue();

		const std::vector<std::string> key4 = data.getValue("key4");
		std::cout << std::endl << "show key4: " << "\n";
		for (int i = 0; i < key4.size(); i++) {
			std::cout << "key4 "<< i << ": " <<  key4[i] << "\n";
		}
		std::cout << "\n" << "key4 Size:" <<data.getSize("key4") << "\n";
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	// invalid file
	try {
		MyYAML data("test222.yaml");
		std::cout << "show all keys: " << "\n";
		data.debugAllKeyAndValue();
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	try {
		MyYAML data("test222.yam");
		std::cout << "show all keys: " << "\n";
		data.debugAllKeyAndValue();
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}