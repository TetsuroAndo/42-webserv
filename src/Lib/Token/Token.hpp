#pragma once

#include "Random.hpp"
#include <string>

class Token {
public:
	static Token &getInstance();

	std::string
	genToken(size_t length = 32,
			 const std::string &charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghij"
										  "klmnopqrstuvwxyz0123456789=-_.");

private:
	Token();
	Token(const Token &);
	Token &operator=(const Token &);
	~Token();

	/// @brief 指定された文字集合から非ドット文字を取得する
	char _getNonDotChar(const std::string &charset);
	/// @brief 指定された文字集合から連続ドットを避けて次の文字を取得する
	char _getNextChar(const std::string &charset, const std::string &token);

	Random &_random;
};
