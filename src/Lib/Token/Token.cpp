#include "Token.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace {
// 連続ドットが生成された場合のリトライ上限
const size_t RETRY_LIMIT = 3;
} // namespace

Token::Token() : _random(Random::getInstance()) {}

Token::~Token() {}

/**
 * @brief Tokenシングルトンインスタンスを取得
 * @note C++98環境ではスレッドセーフではないため、
 *       このメソッドは必ずmain関数の開始時など、
 *       スレッドが分岐するより前に一度呼び出して初期化を完了させること。
 *       これにより、競合状態を回避できる。
 */
Token &Token::getInstance() {
	static Token instance;
	return instance;
}

/// @brief 非ドット文字を取得する（フォールバック処理）
char Token::_getNonDotChar(const std::string &charset) {
	std::vector< size_t > nonDotIndices;
	for (size_t j = 0; j < charset.length(); ++j) {
		if (charset[j] != '.') {
			nonDotIndices.push_back(j);
		}
	}
	if (nonDotIndices.empty()) {
		throw std::runtime_error(
			"GenerateToken: No non-dot character found in charset");
	}
	size_t idx = _random.uniformRand(0, nonDotIndices.size() - 1);
	return charset[nonDotIndices[idx]];
}

/// @brief 連続ドットを避けて次の文字を取得する
char Token::_getNextChar(const std::string &charset, const std::string &token) {
	size_t retryCount = 0;
	while (retryCount < RETRY_LIMIT) {
		char newChar = charset[_random.uniformRand(0, charset.length() - 1)];
		if (newChar != '.' || token.empty() ||
			token[token.length() - 1] != '.') {
			return newChar;
		}
		++retryCount;
	}

	// リトライ上限に達した場合は、非ドット文字を確実に取得
	return _getNonDotChar(charset);
}

/**
 * @brief トークンを生成する
 * @param length トークンの長さ
 * @param charset トークンの文字集合
 * @return 生成されたトークン
 * @throw std::runtime_error charsetが空、または"."のみの場合
 */
std::string Token::genToken(const size_t length, const std::string &charset) {
	if (charset.empty()) {
		throw std::runtime_error("GenerateToken: Charset must not be empty");
	}
	// charsetがすべて"."の場合、無限ループするため
	if (charset.find_first_not_of('.') == std::string::npos) {
		throw std::runtime_error(
			"GenerateToken: Invalid charset: leads to infinite loop");
	}

	std::string token;
	token.reserve(length);
	for (size_t i = 0; i < length; ++i) {
		char newChar = _getNextChar(charset, token);
		token += newChar;
	}
	return token;
}
