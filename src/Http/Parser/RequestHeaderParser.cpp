#include "../Core/HttpStatus.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "RequestHeaderParser.hpp"
#include "ParseResult.hpp"
#include <sstream>

ParseResult RequestHeaderParser::parse(HttpRequest& request, const std::string &headerBlock, int &errorCode) {
	size_t size = headerBlock.size();
	size_t lineStart = 0;
	while (lineStart < size) {
		size_t nl = headerBlock.find('\n', lineStart);
		if (nl == std::string::npos) nl = size;
		size_t lineEnd = nl;
		if (lineEnd > lineStart && headerBlock[lineEnd - 1] == '\r')
			--lineEnd;

		// 空行ならスキップ（\r\n\r\nで分離済み想定なのでERRORにしない）
		if (lineEnd == lineStart) {
			lineStart = nl + 1;
			continue;
		}

		size_t colonPos = headerBlock.find(':', lineStart);
		if (colonPos == std::string::npos || colonPos >= lineEnd || colonPos == lineStart) {
			errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}

		std::string key(headerBlock.substr(lineStart, colonPos - lineStart));
		StringOps::trim(key, " \t");
		if (key.empty()) {
			errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}
		std::string value(headerBlock.substr(colonPos + 1, lineEnd - (colonPos + 1)));
		StringOps::trim(value, " \t");

		if (request.hasHeader(key)) {
			errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}
		request.addHeader(key, value);

		lineStart = nl + 1;
	}
	return PARSE_COMPLETE;
}
