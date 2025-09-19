#include "StringOps.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace // Helper functor structs
{
	struct IsNotDigit {
		inline bool operator()(char c) const {
			return !std::isdigit(static_cast<unsigned char>(c));
		}
	};

	struct CharEqualIgnoreCase {
		inline bool operator()(char lhs, char rhs) const {
			return std::tolower(static_cast<unsigned char>(lhs)) ==
				std::tolower(static_cast<unsigned char>(rhs));
		}
	};

	struct CharToLower {
		inline char operator()(char c) const {
			return std::tolower(static_cast<unsigned char>(c));
		}
	};
}

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
	if (str.empty()) return false;
	return std::find_if(str.begin(), str.end(), IsNotDigit()) == str.end();
}

/**
 * @brief 文字列が等しいか（大文字と小文字を区別しない）を比較する
 */
bool equalsIgnoreCase(const std::string& a, const std::string& b) {
	return a.size() == b.size() &&
		   std::equal(a.begin(), a.end(), b.begin(), CharEqualIgnoreCase());
}

/**
 * @brief 文字列の前後の空白を削除する
 */
void StringOps::trim(std::string &s, const std::string &chars) {
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

/**
 * @brief 文字列を小文字に変換する
 */
void StringOps::toLower(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), CharToLower());
}

/**
 * @brief 文字列を指定した区切り文字で分割する
 */
std::vector<std::string> StringOps::split(const std::string &str,
										const std::string &delimiter) {
	std::vector<std::string> tokens;
	size_t start = 0;
	size_t end = str.find(delimiter);
	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + delimiter.length();
		end = str.find(delimiter, start);
	}
	return tokens;
}

/**
 * @brief 文字列をsize_tに変換する
 * @throw std::invalid_argument 変換できない文字が含まれる場合
 * @throw std::out_of_range      数値がsize_tの範囲を超える場合
 */
inline size_t toSize_t(const std::string &str) {
	std::stringstream ss(str);
	size_t res;
	ss >> res;
	if (ss.fail() || !ss.eof()) {
		throw std::invalid_argument("Invalid conversion to size_t");
	}
	return res;
}
