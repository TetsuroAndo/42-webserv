#pragma once

#include <string>
#include <vector>
#include <sstream> // for toString

namespace StringOps {

	bool startsWith(const std::string &str, const std::string &prefix);
	bool endsWith(const std::string &str, const std::string &suffix);
	bool isNumber(const std::string &str);
	bool equalsIgnoreCase(const std::string& a, const std::string& b);

	int startCharCount(const std::string &str, const char c);
	int startCharCount(const std::string &str, const std::string &chars);
	bool isOnlyCharLine(const std::string &line, const char delimiter);
	bool isOnlyCharLine(const std::string &line, const std::string &delimiters);

	void trim(std::string &s, const std::string &chars = " \t\n\r\f\v");
	void toLower(std::string &str);
	std::vector<std::string> split(const std::string &str,
							   const std::string &delimiter);
	size_t toSize_t(const std::string &str);

	bool hexStrToSize(const char* str, size_t len, size_t& result);
	bool decStrToSize(const std::string& str, size_t& result);

	template <typename T>
	std::string toString(const T &value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

}
