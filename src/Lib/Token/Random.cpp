#include "Random.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <vector>

namespace {
const size_t RANDOM_BUFFER_SIZE = 4096;
const size_t BYTE_RANGE = 256; // unsigned charの取り得る値の数 (0-255)
} // namespace

Random::Random()
	: _urandomFd(-1), _buffer(RANDOM_BUFFER_SIZE), _bufPos(RANDOM_BUFFER_SIZE) {
	_urandomFd = open("/dev/urandom", O_RDONLY);
	if (_urandomFd < 0) {
		int err = errno;
		throw std::runtime_error("Open /dev/urandom failed: " +
								 std::string(strerror(err)));
	}
}

Random::~Random() {
	if (_urandomFd >= 0) {
		close(_urandomFd);
		_urandomFd = -1;
	}
}

/**
 * @brief Randomシングルトンインスタンスを取得
 * @note C++98環境ではスレッドセーフではないため、
 *       このメソッドは必ずmain関数の開始時など、
 *       スレッドが分岐するより前に一度呼び出して初期化を完了させること。
 *       これにより、競合状態を回避できる。
 */
Random &Random::getInstance() {
	static Random instance;
	return instance;
}

/// @brief /dev/urandomをreadするヘルパー
ssize_t Random::_readRandomBytes(unsigned char *buf, size_t size) {
	ssize_t totalRead = 0;
	while (totalRead < static_cast< ssize_t >(size)) {
		ssize_t bytesRead = read(_urandomFd, buf + totalRead, size - totalRead);
		if (bytesRead < 0) {
			int err = errno;
			if (err == EINTR) { // シグナルによる中断の場合はリトライ
				continue;
			}
			throw std::runtime_error("Read from /dev/urandom failed: " +
									 std::string(strerror(err)));
		}
		// EOF (通常/dev/urandomでは発生しないが、念のため)
		if (bytesRead == 0) {
			// 部分読み込みの場合は例外を投げる
			if (totalRead < static_cast< ssize_t >(size)) {
				throw std::runtime_error(
					"Read from /dev/urandom failed: Unexpected EOF");
			}
			return totalRead;
		}
		totalRead += bytesRead;
	}
	return totalRead;
}

/// @brief バッファからバイトを取得し、必要に応じてリフィル
unsigned char Random::getRandomByte() {
	// バッファが空または使い切った場合はリフィル
	if (_bufPos >= _buffer.size()) {
		_readRandomBytes(&_buffer[0], RANDOM_BUFFER_SIZE);
		_bufPos = 0;
	}
	// バッファからバイトを返して位置を進める
	return _buffer[_bufPos++];
}

/// @brief 指定された範囲の一様分布の乱数を生成
size_t Random::uniformRand(size_t min, size_t max) {
	if (min > max) {
		std::swap(min, max);
	}
	const size_t range = max - min + 1;
	if (range == 1 || min == max) {
		return min;
	}

	// 2のべき乗の場合は、ビットマスクで処理
	if ((range & (range - 1)) == 0) {
		unsigned char byte = getRandomByte();
		return (static_cast< size_t >(byte) & (range - 1)) + min;
	}

	// rejection sampling で一様分布の乱数を生成
	if (range > BYTE_RANGE) {
		throw std::runtime_error(
			"Random::uniformRand: Range too large for single-byte sampling");
	}
	const size_t limit = (BYTE_RANGE / range) * range;
	size_t result;
	do {
		unsigned char byte = getRandomByte();
		result = static_cast< size_t >(byte);
	} while (result >= limit);
	return (result % range) + min;
}
