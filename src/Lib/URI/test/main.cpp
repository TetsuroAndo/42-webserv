#include "../uri.h"
#include <iostream>

int main(int ac, char **av) {
	if (ac != 3)
		return 1;
	if (std::string(av[1]) == "encodeURI") {
		std::string str = av[2];
		std::string encoded = ::encodeURI(str);
		std::cout << encoded << std::endl;
	} else if (std::string(av[1]) == "decodeURI") {
		std::string str = av[2];
		std::string decoded = ::decodeURI(str);
		std::cout << decoded << std::endl;
	} else if (std::string(av[1]) == "encodeURIComponent") {
		std::string str = av[2];
		std::string encoded = ::encodeURIComponent(str);
		std::cout << encoded << std::endl;
	} else if (std::string(av[1]) == "decodeURIComponent") {
		std::string str = av[2];
		std::string decoded = ::decodeURIComponent(str);
		std::cout << decoded << std::endl;
	} else
		return 1;
	return 0;
}
