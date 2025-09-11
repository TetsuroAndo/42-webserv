#include "uri.hpp"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

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
