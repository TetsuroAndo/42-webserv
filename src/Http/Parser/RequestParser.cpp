#include "RequestParser.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpStatus.hpp"
#include "ParseResult.hpp"

RequestParser::RequestParser() { reset(); }

RequestParser::~RequestParser() {}

void RequestParser::reset() {
	_state = STATE_REQUEST_LINE;
	_errorCode = 0;
}

int RequestParser::getErrorCode() const { return _errorCode; }

bool RequestParser::isComplete() const { return _state == STATE_COMPLETE; }

ParseResult RequestParser::parse(HttpRequest &request, std::string &buffer) {
	LOG(DEBUG) << "RequestParser::parse called"
			   << attr("buffer_size", buffer.length());
	bool stateChanged = true;
	while (stateChanged) {
		stateChanged = false;

		switch (_state) {
		case STATE_REQUEST_LINE: {
			const size_t crlfPos = buffer.find("\r\n");
			if (crlfPos == std::string::npos)
				return PARSE_INCOMPLETE;

			if (buffer.begin() == buffer.begin() + crlfPos) {
				_errorCode = HttpStatus::BAD_REQUEST;
				LOG(WARNING) << "Parse error: Empty request line"
							 << attr("error_code", _errorCode);
				return PARSE_ERROR;
			}

			std::string line(buffer.begin(), buffer.begin() + crlfPos);
			buffer.erase(0, crlfPos + 2);
			if (_lineParser.parse(request, line, _errorCode) == PARSE_ERROR) {
				LOG(WARNING) << "Failed to parse request line: " << line
							 << attr("error_code", _errorCode);
				return PARSE_ERROR;
			}
			_state = STATE_HEADERS;
			stateChanged = true;
			break;
		}
		case STATE_HEADERS: {
			const size_t headerEndPos = buffer.find("\r\n\r\n");
			if (headerEndPos == std::string::npos) {
				return PARSE_INCOMPLETE;
			}

			std::string headerBlock(buffer.begin(),
									buffer.begin() + headerEndPos);
			buffer.erase(0, headerEndPos + 4);
			if (_headerParser.parse(request, headerBlock, _errorCode) ==
				PARSE_ERROR) {
				LOG(WARNING) << "Failed to parse header block"
							 << attr("error_code", _errorCode);
				return PARSE_ERROR;
			}
			_state = STATE_BODY;
			stateChanged = true;
			break;
		}
		case STATE_BODY: {

			ParseResult res;
			const size_t consumed =
				_bodyParser.parse(request, buffer, _errorCode, res);

			if (consumed > 0) {
				buffer.erase(0, consumed);
			}

			if (request.getBody().length() > request.getMaxBodySize()) {
				_errorCode = HttpStatus::PAYLOAD_TOO_LARGE;
				LOG(WARNING) << "Parse error: Payload too large"
							 << attr("size", buffer.length())
							 << attr("max_size", request.getMaxBodySize());
				return PARSE_ERROR;
			}

			if (res == PARSE_COMPLETE) {
				_state = STATE_COMPLETE;
				LOG(DEBUG) << "Request parsing complete.";
				return PARSE_COMPLETE;
			}
			return PARSE_INCOMPLETE;
		}
		case STATE_COMPLETE:
			return PARSE_COMPLETE;
		}
	}
	return PARSE_INCOMPLETE;
}
