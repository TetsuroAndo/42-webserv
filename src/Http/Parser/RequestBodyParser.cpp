#include "RequestBodyParser.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpRequest.hpp"
#include "../Core/HttpStatus.hpp"
#include <algorithm>
#include <cstring>
#include <ctime>

RequestBodyParser::RequestBodyParser() { reset(); }

RequestBodyParser::~RequestBodyParser() {}

void RequestBodyParser::reset() {
	_state = UNINITIALIZED;
	_contentLengthRemaining = 0;
	_chunkSize = 0;
	_lastReceiveTime = time(NULL);
}

void RequestBodyParser::init(const HttpRequest &request, int &errorCode) {
	if (request.hasHeader("Transfer-Encoding")) {
		const std::string &encoding = request.getHeader("Transfer-Encoding");
		if (encoding == "chunked") {
			if (request.hasHeader("Content-Length")) {
				errorCode = HttpStatus::BAD_REQUEST;
				return;
			}
			_state = CHUNKED_SIZE;
		} else {
			errorCode = HttpStatus::NOT_IMPLEMENTED;
		}
	} else if (request.hasHeader("Content-Length")) {
		const std::string &lenStr = request.getHeader("Content-Length");
		if (!StringOps::decStrToSize(lenStr, _contentLengthRemaining)) {
			errorCode = HttpStatus::BAD_REQUEST;
			return;
		}

		if (_contentLengthRemaining == 0) {
			_state = COMPLETE;
		} else {
			_state = IDENTITY;
		}
	} else {
		_state = COMPLETE;
	}
}

size_t RequestBodyParser::parse(HttpRequest &request, const std::string &buffer,
								int &errorCode, ParseResult &result) {
	if (!buffer.empty()) {
		_lastReceiveTime = time(NULL);
	}

	if (_state == UNINITIALIZED) {
		init(request, errorCode);
		if (errorCode != 0) {
			result = PARSE_ERROR;
			return 0;
		}
	}

	if (_state == COMPLETE) {
		result = PARSE_COMPLETE;
		return 0;
	}

	if (_state == IDENTITY) {
		return parseIdentity(request, buffer, result);
	}
	if (_state >= CHUNKED_SIZE && _state <= CHUNKED_CRLF) {
		return parseChunked(request, buffer, errorCode, result);
	}

	result = PARSE_INCOMPLETE;
	return 0;
}

size_t RequestBodyParser::parseIdentity(HttpRequest &request,
										const std::string &buffer,
										ParseResult &result) {
	const size_t toRead =
		std::min< size_t >(buffer.length(), _contentLengthRemaining);

	if (toRead == 0) {
		result = PARSE_INCOMPLETE;
		return 0;
	}

	request.appendBody(buffer.c_str(), toRead);
	_contentLengthRemaining -= toRead;

	if (_contentLengthRemaining == 0) {
		_state = COMPLETE;
		result = PARSE_COMPLETE;
	} else {
		result = PARSE_INCOMPLETE;
	}

	return toRead;
}

size_t RequestBodyParser::parseChunked(HttpRequest &request,
									   const std::string &buffer,
									   int &errorCode, ParseResult &result) {
	const int timeoutSeconds =
		10; // TODO: ちゃんとしたタイムアウト時間を設定。暫定Timeout値
	size_t offset = 0;
	result = PARSE_INCOMPLETE;

	while (offset < buffer.length()) {
		const time_t now = time(NULL);
		if (now - _lastReceiveTime > timeoutSeconds) {
			errorCode = HttpStatus::REQUEST_TIMEOUT;
			result = PARSE_ERROR;
			return offset;
		}

		if (_state == CHUNKED_SIZE) {
			const size_t crlfPos = buffer.find("\r\n", offset);
			if (crlfPos == std::string::npos)
				return offset;

			const char *sizeLineStart = buffer.c_str() + offset;
			size_t sizeLineLen = crlfPos - offset;

			const void *semiPosPtr = memchr(sizeLineStart, ';', sizeLineLen);
			if (semiPosPtr != NULL) {
				sizeLineLen =
					static_cast< const char * >(semiPosPtr) - sizeLineStart;
			}

			if (!StringOps::hexStrToSize(sizeLineStart, sizeLineLen,
										 _chunkSize)) {
				errorCode = HttpStatus::BAD_REQUEST;
				result = PARSE_ERROR;
				return offset;
			}

			offset = crlfPos + 2;
			if (_chunkSize == 0) {
				_state = CHUNKED_CRLF;
			} else {
				_state = CHUNKED_DATA;
			}
		}

		if (_state == CHUNKED_DATA) {
			if (buffer.length() - offset < _chunkSize + 2)
				return offset;
			request.appendBody(buffer.c_str() + offset, _chunkSize);
			if (!(buffer[offset + _chunkSize] == '\r' &&
				  buffer[offset + _chunkSize + 1] == '\n')) {
				errorCode = HttpStatus::BAD_REQUEST;
				result = PARSE_ERROR;
				return offset;
			}
			offset += _chunkSize + 2;
			_state = CHUNKED_SIZE;
		}

		if (_state == CHUNKED_CRLF) {
			size_t trailerOffset = offset;
			while (true) {
				const size_t crlfPos = buffer.find("\r\n", trailerOffset);
				if (crlfPos == std::string::npos)
					return offset;

				if (crlfPos == trailerOffset) {
					offset = trailerOffset + 2;
					_state = COMPLETE;
					result = PARSE_COMPLETE;
					return offset;
				}
				trailerOffset = crlfPos + 2;
			}
		}
	}
	return offset;
}
