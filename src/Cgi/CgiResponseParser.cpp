#include "CgiResponseParser.hpp"
#include <istream>
#include <sstream>

CgiResponseParser::CgiResponseParser()
	: _statusCode(200), _statusMessage("OK"), _headers(), _body() {}

CgiResponseParser::~CgiResponseParser() {}

void CgiResponseParser::parse(const std::string &rawResponse) {
	(void)rawResponse;
	// TODO: Implement CGI response parsing (Part C)
	// This is intentionally left empty for Part C implementation
}

void CgiResponseParser::setResponse(HttpResponse &httpResponse) {
	(void)httpResponse;
	// TODO: Implement response setting (Part C)
	// This is intentionally left empty for Part C implementation
}

void CgiResponseParser::_parseHeaders(const std::string &headerBlock) {
	(void)headerBlock;
	// TODO: Implement header parsing (Part C)
	// This is intentionally left empty for Part C implementation
}
