#include "HttpStatus.hpp"
#include <string>
#include <map>

const std::string HttpStatus::getReason(int code) {
	std::map<int, std::string>::const_iterator it = statusReasons.find(code);
	if (it != statusReasons.end())
		return it->second;
	return "Internal Server Error";
}
