#include "../Core/HttpStatus.hpp"
#include "ParseResult.hpp"
#include "RequestHeaderParser.hpp"

ParseResult RequestHeaderParser::parse(HttpRequest& request, const std::string &headerBlock, int &errorCode) {
	size_t size = headerBlock.size();
	size_t lineStart = 0;
	while (lineStart < size) {
		size_t nl = headerBlock.find('\n', lineStart);
		if (nl == std::string::npos) nl = size;
		size_t lineEnd = nl;
		if (lineEnd > lineStart && headerBlock[lineEnd - 1] == '\r')
			--lineEnd;

		if (lineEnd > lineStart) {
			size_t colonPos = headerBlock.find(':', lineStart);
			if (colonPos == std::string::npos || colonPos >= lineEnd) {
				errorCode = HttpStatus::BAD_REQUEST;
				return PARSE_ERROR;
			}

			size_t keyStart = lineStart;
			size_t keyEnd = colonPos;
			while (keyStart < keyEnd && (headerBlock[keyStart] == ' ' || headerBlock[keyStart] == '\t')) keyStart++;
			while (keyEnd > keyStart && (headerBlock[keyEnd - 1] == ' ' || headerBlock[keyEnd - 1] == '\t')) keyEnd--;
			size_t keyLen = keyEnd - keyStart;
			if (keyLen == 0) {
				errorCode = HttpStatus::BAD_REQUEST;
				return PARSE_ERROR;
			}

			size_t valueStart = colonPos + 1;
			size_t valueEnd = lineEnd;
			while (valueStart < valueEnd && (headerBlock[valueStart] == ' ' || headerBlock[valueStart] == '\t')) valueStart++;
			while (valueEnd > valueStart && (headerBlock[valueEnd - 1] == ' ' || headerBlock[valueEnd - 1] == '\t')) valueEnd--;
			size_t valueLen = valueEnd - valueStart;

			const char* keyPtr = headerBlock.c_str() + keyStart;
			if (request.hasHeader(keyPtr, keyLen)) {
				errorCode = HttpStatus::BAD_REQUEST;
				return PARSE_ERROR;
			}
			request.addHeader(keyPtr, keyLen, headerBlock.c_str() + valueStart, valueLen);
		}

		lineStart = nl + 1;
	}
	return PARSE_COMPLETE;
}
