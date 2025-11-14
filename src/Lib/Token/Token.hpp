#pragma once

#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>

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

	/// @brief /dev/urandomを読み込み乱数をバッファに格納する
	ssize_t _readRandomBytes(unsigned char *buf, size_t size);
	/// @brief バッファから乱数バイトを取得する/バッファが空の場合はリフィルする
	unsigned char _getRandomByte();
	/// @brief 指定された範囲の一様分布の乱数を生成する
	size_t _uniformRand(size_t min, size_t max);
	/// @brief
	/// 指定された文字集合から非ドット文字を取得する(フォールバック処理のため)
	char _getNonDotChar(const std::string &charset);
	/// @brief 指定された文字集合から連続ドットを避けて次の文字を取得する
	char _getNextChar(const std::string &charset, const std::string &token);

	// /dev/urandomのファイルディスクリプタ
	int _urandomFd;

	std::vector< unsigned char > _buffer;
	size_t _bufPos;
};
