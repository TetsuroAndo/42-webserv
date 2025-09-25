// src/Lib/MyYAML/test/main.cpp
#include "../MyYAML.hpp"
#include <iostream>

int main() {
	try {
		const MyYAML *yaml = new MyYAML("test.yaml"); // テスト用YAMLファイル
		if (yaml->_data) {
			yaml->_data->print(0); // ノード構造を表示
			std::cout << yaml->_data->size() << std::endl;
		} else {
			std::cout << "data is null" << std::endl;
		}
		delete yaml;
	} catch (const std::exception &e) {
		std::cerr << "例外: " << e.what() << std::endl;
	}
	return 0;
}
