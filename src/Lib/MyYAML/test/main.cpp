// src/Lib/MyYAML/test/main.cpp
#include "../MyYAML.hpp"
#include <iostream>
#include <memory>

int main() {
	try {
		std::auto_ptr<MyYAML> yaml(new MyYAML("test.yaml")); // テスト用YAMLファイル
		yaml->getData().print(0); // ノード構造を表示
		std::cout << "最上層のサイズ" << yaml->getData().size() << std::endl;
	} catch (const std::exception &e) {
		std::cerr << "例外: " << e.what() << std::endl;
	}
	return 0;
}
