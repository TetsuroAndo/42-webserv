#pragma once

#include <string>
#include <vector>

namespace StringOps {
	void trim(std::string &s, const std::string &chars = " \t\n\r\f\v");
	bool startsWith(const std::string &str, const std::string &prefix);
	bool endsWith(const std::string &str, const std::string &suffix);
	void toLower(std::string &str);
	std::vector<std::string> split(const std::string &str,
								   const std::string &delimiter);
}
