#pragma once

#include <iostream>
#include <string>
#include <vector>

class Token {
public:
	Token();
	~Token();

	std::string genToken(size_t length = 32);
};
