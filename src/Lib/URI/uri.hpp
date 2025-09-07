#pragma once

#include <string>

namespace URI {
	std::string decodeURI(const std::string &str);
	std::string decodeURIComponent(const std::string &str);
	std::string encodeURI(const std::string &str);
	std::string encodeURIComponent(const std::string &str);
}
