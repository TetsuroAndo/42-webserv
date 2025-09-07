#include <cctype>  // std::isalnum
#include <iomanip> // std::hex, std::setw, std::setfill
#include <iostream>
#include <sstream> // std::ostringstream

/**
 * @brief URLデコードを行う
 * https://triple-underscore.github.io/rfc-others/RFC3986-ja.html#section-2.1
 * @param str デコード対象の文字列
 * @return std::string デコード後の文字列 (UTF-8)
 */
std::string decodeURIComponent(const std::string &str) {
	std::ostringstream decoded;

	for (std::size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%' && i + 2 < str.length()) {
			// '%' の後ろから2文字を切り出す
			std::string hex_val = str.substr(i + 1, 2);
			std::istringstream iss(hex_val);
			int c = 0;
			if (iss >> std::hex >> c) {
				decoded << static_cast<unsigned char>(c);
				i += 2;
			} else {
				decoded << str[i];
			}
		} else if (str[i] == '+') {
			decoded << ' ';
		} else {
			decoded << str[i];
		}
	}
	return decoded.str();
}
