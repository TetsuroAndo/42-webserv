#include <cctype>  // std::isalnum
#include <iomanip> // std::hex, std::setw, std::setfill
#include <iostream>
#include <sstream> // std::ostringstream

/**
 * @brief URLエンコーディング（パーセントエンコーディング）を行う
 * https://triple-underscore.github.io/rfc-others/RFC3986-ja.html#section-2.1
 * @param str エンコード対象の文字列 (UTF-8)
 * @return std::string エンコード後の文字列
 */
std::string encodeURIComponent(const std::string &str) {
	std::ostringstream encoded;
	encoded.fill('0');
	encoded << std::hex << std::uppercase;

	// encodeURIでエンコードしない文字集合
	const std::string unescaped = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqr"
								  "stuvwxyz0123456789-_.!~*'()";

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
