#include "Token.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>

Token::Token() { std::srand(static_cast< unsigned int >(std::time(NULL))); }

Token::~Token() {}

namespace {
long uniformRand(long min, long max) {
	if (min > max) {
		std::swap(min, max);
	}
	const long range = max - min + 1;
	const long randMax = RAND_MAX;
	const long limit = randMax - (randMax + 1) % range;
	long result;
	do {
		result = std::rand();
	} while (result > limit);
	return result % range + min;
}
} // namespace

/**
 * @brief 新しいセッショントークンを生成します。
 * @param length トークンの長さ。最低でも32バイト以上を推奨。
 * @return 生成されたランダムなセッショントークン。
 */
std::string Token::genToken(const size_t length) {
	const std::string charset =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=-_.";
	std::string token;
	token.reserve(length);
	for (size_t i = 0; i < length; ++i) {
		char newChar;
		do {
			newChar = charset[uniformRand(0, charset.length() - 1)];
		} while (newChar == '.' && !token.empty() &&
				 token[token.length() - 1] == '.');
		token += newChar;
	}
	return token;
}

/**
 * @brief 新しいトークンを生成します。
 * @param length トークンの長さ。
 * @return 生成されたランダムなトークン。
 */
std::string Token::genToken(const size_t length, const std::string &charset) {
	std::string token;
	token.reserve(length);
	for (size_t i = 0; i < length; ++i) {
		char newChar;
		do {
			newChar = charset[uniformRand(0, charset.length() - 1)];
		} while (newChar == '.' && !token.empty() &&
				 token[token.length() - 1] == '.');
		token += newChar;
	}
	return token;
}
