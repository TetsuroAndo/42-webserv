#pragma once

#include <iostream>
#include <string>
#include <vector>

class Token {
public:
	Token();
	~Token();

	static std::string genToken(size_t length = 32);
	static std::string genToken(size_t length, const std::string &charset);
};
