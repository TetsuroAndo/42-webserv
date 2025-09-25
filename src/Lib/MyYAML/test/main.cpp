// src/Lib/MyYAML/test/main.cpp
#include "../MyYAML.hpp"
#include <iostream>

int main() {
	try {
		const MyYAML *yaml = new MyYAML("test.yaml"); // テスト用YAMLファイル
		if (yaml->data) {
			yaml->data->print(0); // ノード構造を表示
		} else {
			std::cout << "data is null" << std::endl;
		}
		delete yaml;
	} catch (const std::exception &e) {
		std::cerr << "例外: " << e.what() << std::endl;
	}
	return 0;
}
