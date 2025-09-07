#include "uri.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

/**
 * @brief URLエンコーディング（パーセントエンコーディング）を行う
 * 参考:
 * https://triple-underscore.github.io/rfc-others/RFC3986-ja.html#section-2.1
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURI
 * @param str エンコード対象の文字列 (UTF-8)
 * @return std::string エンコード後の文字列
 */
std::string URI::encodeURI(const std::string &str) {
	std::ostringstream encoded;
	encoded.fill('0');
	encoded << std::hex << std::uppercase;

	const std::string unescaped = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
								  "abcdefghijklmnopqrstuvwxyz"
								  "0123456789"
								  "-._~"		 // 非予約文字
								  ":/?#@"		 // gen-delims
								  "!$&'()*+,;="; // sub-delims

	for (size_t i = 0; i < str.length(); ++i) {
		unsigned char c = str[i];
		if (unescaped.find(c) != std::string::npos) {
			encoded << c;
		} else {
			encoded << '%' << std::setw(2) << std::setfill('0')
					<< static_cast<int>(c);
		}
	}
	return encoded.str();
}
