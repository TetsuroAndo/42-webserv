#include "Token.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

Token::Token() {
	std::srand(static_cast<unsigned int>(std::time(NULL)));
}
Token::~Token() {}

namespace {
	long uniformRand(long min, long max) {
		if (min > max) {
			std::swap(min, max);
		}
		long range = max - min + 1;
		long rand_max = RAND_MAX;
		long limit = rand_max - (rand_max + 1) % range;
		long result;
		do {
			result = std::rand();
		} while (result > limit);
		return result % range + min;
	}
}

/**
 * @brief 新しいセッショントークンを生成します。
 * @param length トークンの長さ。最低でも32バイト以上を推奨。
 * @return 生成されたランダムなセッショントークン。
 */
std::string Token::genToken(size_t length) {
	const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=-_.";
	std::string token;
	token.reserve(length);
	for (size_t i = 0; i < length; ++i) {
		char new_char;
		do {
			new_char = charset[uniformRand(0, charset.length() - 1)];
		} while (new_char == '.' && !token.empty() && token[token.length() - 1] == '.');
		token += new_char;
	}
	return token;
}
