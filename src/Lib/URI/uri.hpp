#pragma once

#include <string>

class URI {
public:
	std::string decodeURI(const std::string &str);
	std::string decodeURIComponent(const std::string &str);
	std::string encodeURI(const std::string &str);
	std::string encodeURIComponent(const std::string &str);

private:
	std::string _internalEncodeURILogic(const std::string &str,
										const std::string &unescaped) const;
	unsigned char _parseHexByte(const std::string &str, std::size_t pos);
};
