#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpStatus.hpp"
#include "ParseResult.hpp"
#include "RequestParser.hpp"

RequestParser::RequestParser() {
	reset();
}

RequestParser::~RequestParser() {}

void RequestParser::reset() {
	_state = STATE_REQUEST_LINE;
	_errorCode = 0;
}

int RequestParser::getErrorCode() const {
	return _errorCode;
}

bool RequestParser::isComplete() const {
	return _state == STATE_COMPLETE;
}

ParseResult RequestParser::parse(HttpRequest& request, std::string& buffer) {
	bool stateChanged = true;
	while (stateChanged) {
		stateChanged = false;

		switch (_state) {
			case STATE_REQUEST_LINE: {
				size_t crlfPos = buffer.find("\r\n");
				if (crlfPos == std::string::npos) return PARSE_INCOMPLETE;

				if (buffer.begin() == buffer.begin() + crlfPos) {
					_errorCode = HttpStatus::BAD_REQUEST;
					return PARSE_ERROR;
				}

				std::string line(buffer.begin(), buffer.begin() + crlfPos);
				buffer.erase(0, crlfPos + 2);
				if (_lineParser.parse(request, line, _errorCode) == PARSE_ERROR) {
					return PARSE_ERROR;
				}
				_state = STATE_HEADERS;
				stateChanged = true;
				break;
			}
			case STATE_HEADERS: {
				size_t headerEndPos = buffer.find("\r\n\r\n");
				if (headerEndPos == std::string::npos) {
					if (buffer.length() > request.getMaxHeaderSize()) {
						_errorCode = HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE;
						return PARSE_ERROR;
					}
					return PARSE_INCOMPLETE;
				}

				std::string headerBlock(buffer.begin(), buffer.begin() + headerEndPos);
				buffer.erase(0, headerEndPos + 4);
				if (_headerParser.parse(request, headerBlock, _errorCode) == PARSE_ERROR) {
					return PARSE_ERROR;
				}
				_state = STATE_BODY;
				stateChanged = true;
				break;
			}
			case STATE_BODY: {
				if (buffer.length() > request.getMaxBodySize()) {
					_errorCode = HttpStatus::PAYLOAD_TOO_LARGE;
					return PARSE_ERROR;
				}

				ParseResult res;
				size_t consumed = _bodyParser.parse(request, buffer, _errorCode, res);

				if (consumed > 0) {
					buffer.erase(0, consumed);
				}

				if (res == PARSE_COMPLETE) {
					_state = STATE_COMPLETE;
				}
				return res;
			}
			case STATE_COMPLETE:
				return PARSE_COMPLETE;
		}
	}
	return PARSE_INCOMPLETE;
}
