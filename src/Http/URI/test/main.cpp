#include "../uri.hpp"
#include <iostream>

#include <iterator>

int main(int ac, char **av) {
	if (ac != 2)
		return 1;
	const std::string mode(av[1]);
	// 重複している行を削除し、正しい構文で書き直す
	const std::string input((std::istreambuf_iterator<char>(std::cin)),
					  std::istreambuf_iterator<char>());

	std::string out;
	if (mode == "encodeURI") {
		out = URI::encodeURI(input);
	} else if (mode == "decodeURI") {
		out = URI::decodeURI(input);
	} else if (mode == "encodeURIComponent") {
		out = URI::encodeURIComponent(input);
	} else if (mode == "decodeURIComponent") {
		out = URI::decodeURIComponent(input);
	} else {
		return 1;
	}
	std::cout << out;
	return 0;
}
