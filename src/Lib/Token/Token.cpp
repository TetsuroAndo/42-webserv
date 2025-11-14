#include "Token.hpp"

#include <algorithm>
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
		throw std::runtime_error("Failed to open /dev/urandom");
	}
}

Token::~Token() {
	if (_urandom_fd >= 0) {
		close(_urandom_fd);
		_urandom_fd = -1;
	}
}

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
		if (bytes_read <= 0) {
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
		ssize_t bytes_read =
			_readRandomBytes(_buffer.data(), RANDOM_BUFFER_SIZE);

		if (bytes_read != static_cast< ssize_t >(RANDOM_BUFFER_SIZE)) {
			throw std::runtime_error("Failed to read from /dev/urandom");
		}
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

/**
 * @brief トークンを生成する
 * @param length トークンの長さ
 * @param charset トークンの文字集合
 * @return 生成されたトークン
 */
std::string Token::genToken(const size_t length, const std::string &charset) {
	std::string token;
	token.reserve(length);
	for (size_t i = 0; i < length; ++i) {
		char newChar;
		do {
			newChar = charset[_uniformRand(0, charset.length() - 1)];
		} while (newChar == '.' && !token.empty() &&
				 token[token.length() - 1] == '.');
		token += newChar;
	}
	return token;
}
