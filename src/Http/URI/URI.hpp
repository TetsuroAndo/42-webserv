#pragma once

#include <string>

class URI {
public:
	static std::string decodeURI(const std::string &str);
	static std::string decodeURIComponent(const std::string &str);
	static std::string encodeURI(const std::string &str);
	static std::string encodeURIComponent(const std::string &str);

private:
	static std::string _internalEncodeURILogic(const std::string &str,
											   const std::string &unescaped);
	static unsigned char _parseHexByte(const std::string &str, std::size_t pos);
};
