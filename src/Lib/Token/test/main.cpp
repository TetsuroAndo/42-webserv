#include "../Token.hpp"
#include <cassert>
#include <iostream>
#include <set>
#include <string>
#include <vector>

int main() {
	// Test 1: Default length token
	std::string token1 = Token::getInstance().genToken();
	std::cout << "Token1 (default length): " << token1 << std::endl;
	assert(token1.length() == 32);

	// Test 2: Custom length token
	size_t customLength = 64;
	std::string token2 = Token::getInstance().genToken(customLength);
	std::cout << "Token2 (custom length): " << token2 << std::endl;
	assert(token2.length() == customLength);

	// Test 3: Multiple tokens uniqueness
	std::cout << "Generating multiple tokens:" << std::endl;
	std::set< std::string > tokens;
	bool unique = true;
	for (int i = 0; i < 999999; ++i) {
		std::string t = Token::getInstance().genToken();
		if (tokens.count(t)) {
			unique = false;
			break;
		}
		tokens.insert(t);
		if (i % 33333 == 0) {
			std::cout << "Token " << i + 1 << ": " << t << std::endl;
		}
	}
	std::cout << "All 999,999 tokens are unique: " << (unique ? "PASS" : "FAIL")
			  << std::endl;
	assert(unique);

	std::cout << "All tests passed!" << std::endl;
	return 0;
}
