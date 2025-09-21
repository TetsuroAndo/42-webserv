
#include "URI.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

unsigned char _parseHexByte(const std::string &str, std::size_t pos) {
	if (pos + 2 >= str.length()) {
		throw std::invalid_argument("URIError: malformed URI sequence");
	}

	std::string hex_val = str.substr(pos + 1, 2);
	std::istringstream iss(hex_val);
	int v = 0;

	if (hex_val.find_first_not_of("0123456789abcdefABCDEF") !=
		std::string::npos) {
		throw std::invalid_argument("URIError: malformed URI sequence");
	}

	if (!(iss >> std::hex >> v)) {
		throw std::invalid_argument("URIError: malformed URI sequence");
	}
	return static_cast<unsigned char>(v);
}

/**
 * @brief URLデコードを行う
 * https://triple-underscore.github.io/rfc-others/RFC3986-ja.html#section-2.1
 * https://developer.mozilla.org/ja/docs/Web/JavaScript/Reference/Global_Objects/decodeURI
 * @param str デコード対象の文字列
 * @throw std::invalid_argument 不正なエンコードが含まれる場合
 * @return std::string デコード後の文字列 (UTF-8)
 */
std::string URI::decodeURI(const std::string &str) {
	std::ostringstream decoded;
	const std::string reserved = ";,/?:@&=+$#";

	for (std::size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%') {
			unsigned char first = _parseHexByte(str, i);

			// 予約文字はデコードせずにそのまま出力
			if (first < 0x80 &&
				reserved.find(static_cast<char>(first)) != std::string::npos) {
				decoded << str[i] << str[i + 1] << str[i + 2];
				i += 2;
				continue;
			}

			// UTF-8 のマルチバイト長を判定
			std::size_t expected_cont = 0;
			if ((first & 0x80) == 0x00) { // 1-byte sequence (ASCII)
				expected_cont = 0;
			} else if ((first & 0xE0) == 0xC0) { // 2-byte sequence
				expected_cont = 1;
			} else if ((first & 0xF0) == 0xE0) { // 3-byte sequence
				expected_cont = 2;
			} else if ((first & 0xF8) == 0xF0) { // 4-byte sequence
				expected_cont = 3;
			} else { // 不正な開始バイト
				throw std::invalid_argument("URIError: malformed URI sequence");
			}

			std::string bytes;
			bytes += static_cast<char>(first);

			// 続行バイトを検証
			for (std::size_t k = 0; k < expected_cont; ++k) {
				std::size_t pos = i + 3 * (k + 1);
				if (pos >= str.length() || str[pos] != '%') {
					throw std::invalid_argument(
						"URIError: malformed URI sequence");
				}
				unsigned char cont = _parseHexByte(str, pos);
				if ((cont & 0xC0) != 0x80) {
					throw std::invalid_argument(
						"URIError: malformed URI sequence");
				}
				bytes += static_cast<char>(cont);
			}
			decoded << bytes;
			i += 2 + expected_cont * 3;
		} else {
			decoded << str[i];
		}
	}
	return decoded.str();
}

/**
 * @brief URLデコードを行う
 * https://triple-underscore.github.io/rfc-others/RFC3986-ja.html#section-2.1
 * https://developer.mozilla.org/ja/docs/Web/JavaScript/Reference/Global_Objects/decodeURIComponent
 * @param str デコード対象の文字列
 * @return std::string デコード後の文字列 (UTF-8)
 */
std::string URI::decodeURIComponent(const std::string &str) {
	return URI::decodeURI(str);
}

/**
 * @brief URLエンコーディング（パーセントエンコーディング）を行う
 * 参考:
 * https://triple-underscore.github.io/rfc-others/RFC3986-ja.html#section-2.1
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURI
 * @param str エンコード対象の文字列 (UTF-8)
 * @return std::string エンコード後の文字列
 */
std::string URI::encodeURI(const std::string &str) {
	const std::string unescaped = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
								  "abcdefghijklmnopqrstuvwxyz"
								  "0123456789"
								  "-._~"		 // 非予約文字
								  ":/?#@"		 // gen-delims
								  "!$&'()*+,;="; // sub-delims
	return _internalEncodeURILogic(str, unescaped);
}

/**
 * @brief URLエンコーディング（パーセントエンコーディング）を行う
 * 参考:
 * https://triple-underscore.github.io/rfc-others/RFC3986-ja.html#section-2.1
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/encodeURIComponent
 * @param str エンコード対象の文字列 (UTF-8)
 * @return std::string エンコード後の文字列
 */
std::string URI::encodeURIComponent(const std::string &str) {
	const std::string unescaped = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
								  "abcdefghijklmnopqrstuvwxyz"
								  "0123456789"
								  "-_.!~*'()";
	return _internalEncodeURILogic(str, unescaped);
}

std::string URI::_internalEncodeURILogic(const std::string &str,
							 const std::string &unescaped) {
	std::ostringstream encoded;
	encoded.fill('0');
	encoded << std::hex << std::uppercase;

	for (size_t i = 0; i < str.length(); ++i) {
		unsigned char c = static_cast<unsigned char>(str[i]);
		if (unescaped.find(static_cast<char>(c)) != std::string::npos) {
			encoded << static_cast<char>(c);
		} else {
			encoded << '%' << std::setw(2) << std::setfill('0')
					<< static_cast<int>(c);
		}
	}
	return encoded.str();
}
