#include "../Core/HttpStatus.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "RequestParser.hpp"
#include "ParseResult.hpp"
#include <sstream>

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
	bool state_changed = true;
	while (state_changed) {
		state_changed = false;

		switch (_state) {
			case STATE_REQUEST_LINE: {
				size_t crlf_pos = buffer.find("\r\n");
				if (crlf_pos == std::string::npos) return PARSE_INCOMPLETE;

				const char* line_start = buffer.c_str();
				const char* line_end = line_start + crlf_pos;
				if (line_start == line_end) {
					buffer.erase(0, crlf_pos + 2);
					state_changed = true;
					continue;
				}
				std::string line(line_start, line_end);
				buffer.erase(0, crlf_pos + 2);
				if (_lineParser.parse(request, line, _errorCode) == PARSE_ERROR) {
					return PARSE_ERROR;
				}
				_state = STATE_HEADERS;
				state_changed = true;
				break;
			}
			case STATE_HEADERS: {
				size_t header_end_pos = buffer.find("\r\n\r\n");
				if (header_end_pos == std::string::npos) return PARSE_INCOMPLETE;

				const char *headers_start = buffer.c_str();
				const char *headers_end = headers_start + header_end_pos;

				std::istringstream iss(std::string(headers_start, headers_end));
				std::string line;
				while (std::getline(iss, line)) {
					StringOps::trim(line, "\r");
					if (line.empty()) continue;
					if (_headerParser.parse(request, line, _errorCode) == PARSE_ERROR) {
						return PARSE_ERROR;
					}
				}
				_state = STATE_BODY;
				state_changed = true;
				break;
			}
			case STATE_BODY: {
				ParseResult res = _bodyParser.parse(request, buffer, _errorCode);
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
