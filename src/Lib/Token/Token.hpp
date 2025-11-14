// Token.hpp
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

	/// @brief /dev/urandomを読み込むヘルパー
	ssize_t _readRandomBytes(unsigned char *buf, size_t size);
	/// @brief 乱数バイトを取得するヘルパー
	unsigned char _getRandomByte();
	/// @brief 均一な乱数を生成するヘルパー
	size_t _uniformRand(size_t min, size_t max);
	/// @brief 非ドット文字を取得する（フォールバック処理）
	char _getNonDotChar(const std::string &charset);
	/// @brief 連続ドットを避けて次の文字を取得する
	char _getNextChar(const std::string &charset, const std::string &token);

	// /dev/urandomのファイルディスクリプタ
	int _urandom_fd;

	std::vector< unsigned char > _buffer;
	size_t _buffer_pos;
};
