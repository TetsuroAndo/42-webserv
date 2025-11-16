#pragma once

#include <sys/types.h>
#include <vector>

class Random {
public:
	static Random &getInstance();

	/// @brief 指定された範囲の一様分布の乱数を生成する
	size_t uniformRand(size_t min, size_t max);

	/// @brief バッファから乱数バイトを取得する/バッファが空の場合はリフィルする
	unsigned char getRandomByte();

private:
	Random();
	Random(const Random &);
	Random &operator=(const Random &);
	~Random();

	/// @brief /dev/urandomを読み込み乱数をバッファに格納する
	ssize_t _readRandomBytes(unsigned char *buf, size_t size);

	// /dev/urandomのファイルディスクリプタ
	int _urandomFd;

	std::vector< unsigned char > _buffer;
	size_t _bufPos;
};
