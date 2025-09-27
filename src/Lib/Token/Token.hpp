#pragma once

#include <iostream>
#include <string>
#include <vector>

class Token {
public:
	Token();
	~Token();

	static std::string genToken(size_t length = 32);
};
