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
	// 指定された範囲 [min, max] の整数を、より一様に生成する関数
	// uniform_int_distributionの簡易的な代替
	long uniformRand(long min, long max) {
		if (min > max) {
			std::swap(min, max);
		}
		long range = max - min + 1;
		// rand()が返す値の最大値
		long rand_max = RAND_MAX;
		// 偏りが発生しない最大の区切り値を見つける
		// (rand_max + 1) が range で割り切れない場合、余りの部分が偏りの原因になる
		long limit = rand_max - (rand_max + 1) % range;
		long result;
		// 生成された乱数が limit を超えていたら、偏りのある範囲なのでやり直す
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
	// 使用する文字セット（英数字）
	const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789=-_.";
	std::string token;
	token.reserve(length);
	do {
		token.clear();
		for (size_t i = 0; i < length; ++i) {
			token += charset[uniformRand(0, charset.length() - 1)];
		}
	} while (
		token.find("..") != std::string::npos ||
		token.size() != length
	);
	return token;
}
