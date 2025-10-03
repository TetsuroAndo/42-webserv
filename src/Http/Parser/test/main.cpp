#include "RequestBodyParserTest.hpp"
#include "RequestHeaderParserTest.hpp"
#include "RequestLineParserTest.hpp"
#include <iostream>

int main() {
    std::cout << "=== RequestBodyParser Tests ===" << std::endl;
    runAllRequestBodyParserTests();

    std::cout << "\n=== RequestHeaderParser Tests ===" << std::endl;
	runRequestHeaderParserTests();

    std::cout << "\n=== RequestLineParser Tests ===" << std::endl;
	runRequestLineParserTests();

    return 0;
}
