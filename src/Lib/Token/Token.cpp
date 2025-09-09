#include "Token.hpp"
#include <iostream>
#include <string>
#include <random>
#include <vector>

Token::Token() : _generator(std::random_device{}()) {}
Token::~Token() {}

/**
 * @brief 新しいセッショントークンを生成します。
 * @param length トークンの長さ。最低でも32バイト以上を推奨。
 * @return 生成されたランダムなセッショントークン。
 */
std::string Token::genToken(size_t length) {
	// 使用する文字セット（英数字）
	const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=-_.";

	// 文字セットからランダムに選ぶための分布
	std::uniform_int_distribution<int> distribution(0, charset.length() - 1);

	std::string token;
	token.reserve(length);
	for (size_t i = 0; i < length; ++i) {
		token += charset[distribution(_generator)];
	}
	return token;
}
