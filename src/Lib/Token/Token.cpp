#include "Token.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>

namespace {
const size_t RANDOM_BUFFER_SIZE = 128;
const size_t BYTE_RANGE = 256; // unsigned charの取り得る値の数 (0-255)
} // namespace

Token::Token()
	: _urandom_fd(-1), _buffer(RANDOM_BUFFER_SIZE),
	  _buffer_pos(RANDOM_BUFFER_SIZE) {
	_urandom_fd = open("/dev/urandom", O_RDONLY);
	if (_urandom_fd < 0) {
		int err = errno;
		throw std::runtime_error("Open /dev/urandom failed: " +
								 std::string(strerror(err)));
	}
}

Token::~Token() {
	if (_urandom_fd >= 0) {
		close(_urandom_fd);
		_urandom_fd = -1;
	}
}

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

/// @brief /dev/urandomをreadするヘルパー
ssize_t Token::_readRandomBytes(unsigned char *buf, size_t size) {
	ssize_t total_read = 0;
	while (total_read < static_cast< ssize_t >(size)) {
		ssize_t bytes_read =
			read(_urandom_fd, buf + total_read, size - total_read);
		if (bytes_read < 0) {
			int err = errno;
			if (err == EINTR) { // シグナルによる中断の場合はリトライ
				continue;
			}
			throw std::runtime_error("Read from /dev/urandom failed: " +
									 std::string(strerror(err)));
		}
		// EOF (通常/dev/urandomでは発生しないが、念のため)
		if (bytes_read == 0) {
			// 部分読み込みの場合は例外を投げる
			if (total_read < static_cast< ssize_t >(size)) {
				throw std::runtime_error(
					"Read from /dev/urandom failed: Unexpected EOF");
			}
			return total_read;
		}
		total_read += bytes_read;
	}
	return total_read;
}

/// @brief バッファからバイトを取得し、必要に応じてリフィル
unsigned char Token::_getRandomByte() {
	// バッファが空または使い切った場合はリフィル
	if (_buffer_pos >= _buffer.size()) {
		_readRandomBytes(&_buffer[0], RANDOM_BUFFER_SIZE);
		_buffer_pos = 0;
	}
	// バッファからバイトを返して位置を進める
	return _buffer[_buffer_pos++];
}

/// @brief 指定された範囲の一様分布の乱数を生成
size_t Token::_uniformRand(size_t min, size_t max) {
	if (min > max) {
		std::swap(min, max);
	}
	const size_t range = max - min + 1;
	if (range == 1) {
		return min;
	}

	// 2のべき乗の場合は、ビットマスクで処理
	if ((range & (range - 1)) == 0) {
		unsigned char byte = _getRandomByte();
		return (static_cast< size_t >(byte) & (range - 1)) + min;
	}

	// rejection sampling で一様分布の乱数を生成
	const size_t limit = (BYTE_RANGE / range) * range;
	size_t result;
	do {
		unsigned char byte = _getRandomByte();
		result = static_cast< size_t >(byte);
	} while (result >= limit);
	return (result % range) + min;
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
	size_t idx = _uniformRand(0, nonDotIndices.size() - 1);
	return charset[nonDotIndices[idx]];
}

/// @brief 連続ドットを避けて次の文字を取得する
char Token::_getNextChar(const std::string &charset, const std::string &token) {
	// 現実的なリトライ上限の数値として文字セットサイズの10倍を上限とする
	const size_t retryLimit = charset.length() * 10;

	size_t retryCount = 0;
	while (retryCount < retryLimit) {
		char newChar = charset[_uniformRand(0, charset.length() - 1)];
		// 連続ドットでない場合はそのまま返す
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
