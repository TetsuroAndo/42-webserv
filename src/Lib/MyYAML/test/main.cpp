#include "../MyYAML.hpp"
#include "../Node.hpp"

#include <iostream>

int main() {
	Node root(NODE_MAP, "", "root");
	Node seq(NODE_SEQ, "seq", "");
	Node seq2(NODE_SEQ, "seq", "");
	Node map(NODE_MAP, "map", "");
	Node val(NODE_VAL, "val1", "huga");
	Node val2(NODE_VAL, "val2", "huga");
	Node val3(NODE_VAL, "val3", "huga");
	Node val4(NODE_VAL, "val4", "huga");
	Node val5(NODE_VAL, "val5", "huga");
	Node val6(NODE_VAL, "val6", "huga");
	try {
		root.push(&seq);
		root.push(&map);
		map.push(&val);
		map.push(&val2);
		map.push(&val3);
		seq.push(&val4);
		seq.push(&val5);
		seq.push(&seq2);
		seq2.push(&val6);
		root.print(2);
		std::cout << "root is " << (root.isValidNode() ? "valid" : "not valid") << std::endl;
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	try {
		std::map<std::string, Node *> tmpMap = root.getMap();
		Node node = *tmpMap["map"];
		node.print(2);

		tmpMap = tmpMap["map"]->getMap();
		Node tmpVal = *tmpMap["val1"];
		tmpVal.print(2);
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}


	// try {
	// 	MyYAML data("test.yaml");
	// 	std::cout << "show all keys: " << "\n";
	// 	data.debugAllKeyAndValue();
	//
	// 	// const std::vector<std::string> key4 = data.getValue("key4");
	// 	// std::cout << std::endl << "show key4: " << "\n";
	// 	// for (int i = 0; i < key4.size(); i++) {
	// 	// 	std::cout << "key4 "<< i << ": " <<  key4[i] << "\n";
	// 	// }
	// 	// std::cout << "\n" << "key4 Size:" <<data.getSize("key4") << "\n";
	// 	std::cout << std::flush;
	// 	return 0;
	// } catch (std::exception& e) {
	// 	std::cerr << e.what() << std::endl;
	// }
	// // invalid file
	// try {
	// 	MyYAML data("test222.yaml");
	// 	std::cout << "show all keys: " << "\n";
	// 	data.debugAllKeyAndValue();
	// } catch (const std::exception& e) {
	// 	std::cout << e.what() << std::endl;
	// }
	// try {
	// 	MyYAML data("test222.yam");
	// 	std::cout << "show all keys: " << "\n";
	// 	data.debugAllKeyAndValue();
	// } catch (const std::exception& e) {
	// 	std::cout << e.what() << std::endl;
	// }
	return 0;
}
