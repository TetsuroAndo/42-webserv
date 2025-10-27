#pragma once

#include <sstream> // for toString
#include <string>
#include <vector>

namespace StringOps {
bool startsWith(const std::string &str, const std::string &prefix);
bool endsWith(const std::string &str, const std::string &suffix);
bool isNumber(const std::string &str);
bool equalsIgnoreCase(const std::string &a, const std::string &b);

int startCharCount(const std::string &str, const char c);
int startCharCount(const std::string &str, const std::string &chars);
bool isOnlyCharLine(const std::string &line, const char delimiter);
bool isOnlyCharLine(const std::string &line, const std::string &delimiters);

void trim(std::string &s, const std::string &chars = " \t\n\r\f\v");
std::string trim(const std::string &s,
				 const std::string &chars = " \t\n\r\f\v");
void toUpper(std::string &str);
void toLower(std::string &str);
std::string toUpper(const std::string &str);
std::string toLower(const std::string &str);
std::vector< std::string > split(const std::string &str,
								 const std::string &delimiter);
int stringToInt(const std::string &s);
size_t toSizeT(const std::string &str);

bool hexStrToSize(const char *str, size_t len, size_t &result);
bool decStrToSize(const std::string &str, size_t &result);

size_t sizeByteStrToSizeT(const std::string &sizeStr);
unsigned int sizeByteStrToUInt(const std::string &sizeStr);

std::string ipToString(uint32_t ip_addr);

template < typename T > std::string toString(const T &value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}
} // namespace StringOps
