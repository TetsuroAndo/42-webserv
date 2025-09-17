#pragma once

#include <string>
#include <vector>

namespace StringOps {

	bool startsWith(const std::string &str, const std::string &prefix);
	bool endsWith(const std::string &str, const std::string &suffix);
	bool isNumber(const std::string &str);
	bool equalsIgnoreCase(const std::string& a, const std::string& b);

	void trim(std::string &s, const std::string &chars = " \t\n\r\f\v");
	void toLower(std::string &str);
	std::vector<std::string> split(const std::string &str,
								   const std::string &delimiter);
	inline size_t toSize_t(const std::string &str);

	template <typename T>
	std::string toString(const T &value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

}
