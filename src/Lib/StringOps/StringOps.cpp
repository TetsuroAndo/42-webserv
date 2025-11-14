#include "StringOps.hpp"
#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace // Helper functor structs
{
struct IsNotDigit {
	bool operator()(const char c) const {
		return !std::isdigit(static_cast< unsigned char >(c));
	}
};

struct CharEqualIgnoreCase {
	bool operator()(const char lhs, const char rhs) const {
		return std::tolower(static_cast< unsigned char >(lhs)) ==
			   std::tolower(static_cast< unsigned char >(rhs));
	}
};

struct CharToUpper {
	char operator()(const char c) const {
		return std::toupper(static_cast< unsigned char >(c));
	}
};

struct CharToLower {
	char operator()(const char c) const {
		return std::tolower(static_cast< unsigned char >(c));
	}
};

// 16進数の1文字を数値に変換する
bool hexCharToDigit(const char c, unsigned int &digit) {
	if (std::isdigit(c)) {
		digit = c - '0';
		return true;
	}
	if (std::isxdigit(c)) {
		digit = std::tolower(c) - 'a' + 10;
		return true;
	}
	return false;
}

} // namespace

namespace StringOps {
/**
 * @brief 文字列が指定した接頭辞で始まるかどうかを判定する
 */
bool startsWith(const std::string &str, const std::string &prefix) {
	return str.size() >= prefix.size() &&
		   std::equal(prefix.begin(), prefix.end(), str.begin());
}

/**
 * @brief 文字列が指定した接尾辞で終わるかどうかを判定する
 */
bool endsWith(const std::string &str, const std::string &suffix) {
	return str.size() >= suffix.size() &&
		   std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

/**
 * @brief 文字列が数値かどうかを判定する
 */
bool isNumber(const std::string &str) {
	if (str.empty())
		return false;
	return std::find_if(str.begin(), str.end(), IsNotDigit()) == str.end();
}

/**
 * @brief 文字列が等しいか（大文字と小文字を区別しない）を比較する
 */
bool equalsIgnoreCase(const std::string &a, const std::string &b) {
	return a.size() == b.size() &&
		   std::equal(a.begin(), a.end(), b.begin(), CharEqualIgnoreCase());
}

int startCharCount(const std::string &str, const char c) {
	const std::string strC(1, c);
	return startCharCount(str, strC);
}

int startCharCount(const std::string &str, const std::string &chars) {
	int result = 0;
	std::string::const_iterator it = str.begin();
	const std::string::const_iterator itEnd = str.end();
	while (it != itEnd && chars.find(*it) != std::string::npos) {
		result++;
		++it;
	}
	return result;
}

bool isOnlyCharLine(const std::string &line, const char delimiter) {
	const std::string delimiters(1, delimiter);
	return isOnlyCharLine(line, delimiters);
}

bool isOnlyCharLine(const std::string &line, const std::string &delimiters) {
	std::string::const_iterator it = line.begin();
	const std::string::const_iterator itEnd = line.end();
	while (it != itEnd && ' ' == *it) {
		++it;
	}
	if (it == itEnd) {
		return false;
	}
	return delimiters.find(*it) != std::string::npos;
}

/**
 * @brief 文字列の前後の空白を削除する
 */
void trim(std::string &s, const std::string &chars) {
	size_t start = 0;
	while (start < s.size() && chars.find(s[start]) != std::string::npos) {
		++start;
	}
	size_t end = s.size();
	while (end > start && chars.find(s[end - 1]) != std::string::npos) {
		--end;
	}
	s = s.substr(start, end - start);
}

std::string trim(const std::string &s, const std::string &chars) {
	std::string result = s;
	trim(result, chars);
	return result;
}

/**
 * @brief 文字列を大文字に変換する
 */
void toUpper(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), CharToUpper());
}

/**
 * @brief 文字列を小文字に変換する
 */
void toLower(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), CharToLower());
}

/**
 * @brief 大文字に変換した新しい文字列を返す
 */
std::string toUpper(const std::string &str) {
	std::string result = str;
	toUpper(result);
	return result;
}

/**
 * @brief 小文字に変換した新しい文字列を返す
 */
std::string toLower(const std::string &str) {
	std::string result = str;
	toLower(result);
	return result;
}

/**
 * @brief 文字列を指定した区切り文字で分割する
 */
std::vector< std::string > split(const std::string &str,
								 const std::string &delimiter) {
	std::vector< std::string > tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);
	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + delimiter.length();
		end = str.find(delimiter, start);
	}
	tokens.push_back(str.substr(start));
	return tokens;
}

/**
 * @brief 文字列をsize_tに変換する
 * @throw std::invalid_argument 変換できない文字が含まれる場合
 * @throw std::out_of_range      数値がsize_tの範囲を超える場合
 */
size_t toSizeT(const std::string &str) {
	std::stringstream ss(str);
	size_t res;
	ss >> res;
	if (ss.fail() || !ss.eof()) {
		throw std::invalid_argument("Invalid conversion to size_t");
	}
	return res;
}

int stringToInt(const std::string &s) {
	std::istringstream iss(s);
	int i;
	if (!(iss >> i) || !iss.eof()) {
		throw std::runtime_error("invalid integer format");
	}
	return i;
}

bool hexStrToSize(const char *str, const size_t len, size_t &result) {
	result = 0;
	if (len == 0) {
		return false;
	}

	const size_t maxDiv16 = std::numeric_limits< size_t >::max() / 16;
	const size_t maxMod16 = std::numeric_limits< size_t >::max() % 16;

	for (size_t i = 0; i < len; ++i) {
		unsigned int digit;
		if (!hexCharToDigit(str[i], digit)) {
			return false; // 不正な文字
		}

		if (result > maxDiv16 || (result == maxDiv16 && digit > maxMod16)) {
			return false; // オーバーフロー
		}
		result = result * 16 + digit;
	}
	return true;
}

bool decStrToSize(const std::string &str, size_t &result) {
	result = 0;
	if (str.empty()) {
		return false;
	}

	const size_t maxDiv10 = std::numeric_limits< size_t >::max() / 10;
	const size_t maxMod10 = std::numeric_limits< size_t >::max() % 10;

	for (size_t i = 0; i < str.length(); ++i) {
		if (!std::isdigit(str[i])) {
			return false;
		}
		const unsigned int digit = str[i] - '0';

		if (result > maxDiv10 || (result == maxDiv10 && digit > maxMod10)) {
			return false;
		}
		result = result * 10 + digit;
	}
	return true;
}

size_t sizeByteStrToSizeT(const std::string &sizeStr) {
	if (sizeStr.empty()) {
		throw std::runtime_error("Config error: size string is empty.");
	}

	std::string num_part;
	size_t i = 0;

	// 負の数(-記号)のチェックと数字部分の抽出
	while (i < sizeStr.length() && std::isspace(sizeStr[i])) {
		i++;
	}
	if (i < sizeStr.length() && sizeStr[i] == '-') {
		throw std::runtime_error(
			"Config error: size must be a non-negative value in '" + sizeStr +
			"'.");
	}

	// 数字部分の抽出
	while (i < sizeStr.length() && std::isdigit(sizeStr[i])) {
		num_part += sizeStr[i];
		i++;
	}
	if (num_part.empty()) {
		throw std::runtime_error(
			"Config error: invalid size format, missing number in '" + sizeStr +
			"'.");
	}

	// 数字部分をsize_tに変換
	std::stringstream ss(num_part);
	size_t number;
	ss >> number;
	if (ss.fail() || !ss.eof()) {
		throw std::runtime_error(
			"Config error: invalid size number format in '" + sizeStr + "'.");
	}

	// 単位部分の抽出
	const std::string unit_part = toUpper(trim(sizeStr.substr(i)));

	const size_t max_size_t = std::numeric_limits< size_t >::max();
	if (unit_part.empty() || unit_part == "B") {
		return number;
	}
	if (unit_part == "KB") {
		if (number > max_size_t / 1024) {
			throw std::runtime_error("Config error: size value is too large '" +
									 sizeStr + "'.");
		}
		return number * 1024;
	}
	if (unit_part == "MB") {
		if (number > max_size_t / (1024 * 1024)) {
			throw std::runtime_error("Config error: size value is too large '" +
									 sizeStr + "'.");
		}
		return number * 1024 * 1024;
	}
	if (unit_part == "GB") {
		if (number > max_size_t / (1024 * 1024 * 1024)) {
			throw std::runtime_error("Config error: size value is too large '" +
									 sizeStr + "'.");
		}
		return number * 1024 * 1024 * 1024;
	}
	throw std::runtime_error("Config error: unknown size unit '" + unit_part +
							 "'. Use B, KB, MB, or GB.");
}

// clang-format off
std::string ipToString(uint32_t ip_addr) {
	std::stringstream ss;
	ss << ((ip_addr >> 24) & 0xFF) << "."
	   << ((ip_addr >> 16) & 0xFF) << "."
	   << ((ip_addr >> 8) & 0xFF) << "."
	   << (ip_addr & 0xFF);
	return ss.str();
}
// clang-format on

} // namespace StringOps
