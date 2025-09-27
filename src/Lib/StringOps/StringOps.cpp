#include "StringOps.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <limits>

namespace // Helper functor structs
{
	struct IsNotDigit {
		bool operator()(char c) const {
			return !std::isdigit(static_cast<unsigned char>(c));
		}
	};

	struct CharEqualIgnoreCase {
		bool operator()(char lhs, char rhs) const {
			return std::tolower(static_cast<unsigned char>(lhs)) ==
				std::tolower(static_cast<unsigned char>(rhs));
		}
	};

	struct CharToLower {
		char operator()(char c) const {
			return std::tolower(static_cast<unsigned char>(c));
		}
	};

	// 16進数の1文字を数値に変換する
	bool hexCharToDigit(char c, unsigned int& digit) {
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

}

namespace StringOps
{
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

	int startCharCount(const std::string &str, const char c) {
		int result = 0;
		std::string::const_iterator it = str.begin();
		const std::string::const_iterator itEnd = str.end();
		while (it != itEnd && c == *it) {
			result++;
			++it;
		}
		return result;
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
		std::string::const_iterator it = line.begin();
		const std::string::const_iterator itEnd = line.end();
		while (it != itEnd && ' ' == *it) {
			++it;
		}
		if (it == itEnd) {
			return false;
		}
		return delimiter == *it;
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

	/**
	 * @brief 文字列を小文字に変換する
	 */
	void toLower(std::string &str) {
		std::transform(str.begin(), str.end(), str.begin(), CharToLower());
	}

	/**
	 * @brief 文字列を指定した区切り文字で分割する
	 */
	std::vector<std::string> split(const std::string &str,
											const std::string &delimiter) {
		std::vector<std::string> tokens;
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
	size_t toSize_t(const std::string &str) {
		std::stringstream ss(str);
		size_t res;
		ss >> res;
		if (ss.fail() || !ss.eof()) {
			throw std::invalid_argument("Invalid conversion to size_t");
		}
		return res;
	}

	bool hexStrToSize(const char* str, size_t len, size_t& result) {
		result = 0;
		if (len == 0) {
			return false;
		}

		const size_t maxDiv16 = std::numeric_limits<size_t>::max() / 16;
		const size_t maxMod16 = std::numeric_limits<size_t>::max() % 16;

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

	bool decStrToSize(const std::string& str, size_t& result) {
		result = 0;
		if (str.empty()) {
			return false;
		}

		const size_t maxDiv10 = std::numeric_limits<size_t>::max() / 10;
		const size_t maxMod10 = std::numeric_limits<size_t>::max() % 10;

		for (size_t i = 0; i < str.length(); ++i) {
			if (!std::isdigit(str[i])) {
				return false;
			}
			unsigned int digit = str[i] - '0';

			if (result > maxDiv10 || (result == maxDiv10 && digit > maxMod10)) {
				return false;
			}
			result = result * 10 + digit;
		}
		return true;
	}
} // namespace StringOps
