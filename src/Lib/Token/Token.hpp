#pragma once

#include <iostream>
#include <string>
#include <vector>

class Token {
public:
	static Token &getInstance();

	std::string genToken(size_t length = 32);
	std::string genToken(size_t length, const std::string &charset);

private:
	Token();
	Token(const Token &);
	Token &operator=(const Token &);
	~Token();
};
