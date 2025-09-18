#include "../Core/HttpStatus.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "RequestHeaderParser.hpp"
#include "ParseResult.hpp"
#include <sstream>

ParseResult RequestHeaderParser::parse(HttpRequest& request, const std::string& line, int &errorCode) {
	size_t colon_pos = line.find(':');
	if (colon_pos == std::string::npos || colon_pos == 0) {
		errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}
	std::string key = line.substr(0, colon_pos);
	std::string value = line.substr(colon_pos + 1);
	StringOps::trim(key);
	StringOps::trim(value);
	
	if (request.hasHeader(key)) {
		errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}
	request.addHeader(key, value);
	return PARSE_COMPLETE;
}
