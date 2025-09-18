#include "../Core/HttpStatus.hpp"
#include "../Core/HttpRequest.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "RequestBodyParser.hpp"
#include "ParseResult.hpp"
#include <sstream>

RequestBodyParser::RequestBodyParser() : _state(STATE_INIT), _bodySizeRemaining(0), _chunkSize(0) {}
RequestBodyParser::~RequestBodyParser() {}

ParseResult RequestBodyParser::parse(HttpRequest& request, std::string& buffer, int& errorCode) {
	if (_state == STATE_INIT) {
		init(request, errorCode);
		if (errorCode != 0) return PARSE_ERROR;
		if (_state == STATE_COMPLETE) return PARSE_COMPLETE;
	}

	if (_state == STATE_CONTENT_LENGTH) {
		return parseContentLength(request, buffer);
	}
	
	if (_state == STATE_CHUNKED_SIZE || _state == STATE_CHUNKED_DATA) {
		return parseChunked(request, buffer, errorCode);
	}

	return PARSE_COMPLETE;
}

void RequestBodyParser::init(const HttpRequest& request, int& errorCode) {
	if (request.hasHeader("Transfer-Encoding")) {
		if (request.getHeader("Transfer-Encoding") == "chunked") {
			if (request.hasHeader("Content-Length")) {
				errorCode = HttpStatus::BAD_REQUEST;
				return;
			}
			_state = STATE_CHUNKED_SIZE;
		} else {
			errorCode = HttpStatus::NOT_IMPLEMENTED;
		}
	} else if (request.hasHeader("Content-Length")) {
		const std::string& lenStr = request.getHeader("Content-Length");
		std::stringstream ss(lenStr);
		ss >> _bodySizeRemaining;
		if (ss.fail() || !ss.eof()) {
			errorCode = HttpStatus::BAD_REQUEST;
			return;
		}
		_state = _bodySizeRemaining > 0 ? STATE_CONTENT_LENGTH : STATE_COMPLETE;
	} else {
		_state = STATE_COMPLETE; // No body
	}
}

ParseResult RequestBodyParser::parseContentLength(HttpRequest& request, std::string& buffer) {
	if (buffer.length() < _bodySizeRemaining) {
		return PARSE_INCOMPLETE;
	}
	request.appendBody(buffer.substr(0, _bodySizeRemaining));
	buffer.erase(0, _bodySizeRemaining);
	_bodySizeRemaining = 0;
	_state = STATE_COMPLETE;
	return PARSE_COMPLETE;
}

ParseResult RequestBodyParser::parseChunked(HttpRequest& request, std::string& buffer, int& errorCode) {
	// (Chunked parsing logic here... for now, return NOT_IMPLEMENTED as before)
	errorCode = HttpStatus::NOT_IMPLEMENTED;
	return PARSE_ERROR;
}