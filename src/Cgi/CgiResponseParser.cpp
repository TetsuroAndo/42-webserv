#include "CgiResponseParser.hpp"
#include <istream>
#include <sstream>

CgiResponseParser::CgiResponseParser()
	: _statusCode(200), _statusMessage("OK"), _headers(), _body() {}

CgiResponseParser::~CgiResponseParser() {}

void CgiResponseParser::parse(const std::string &rawResponse) {
	(void)rawResponse;
	// TODO: Implement CGI response parsing
	// Parse headers and body from CGI output
}

void CgiResponseParser::setResponse(HttpResponse &httpResponse) {
	(void)httpResponse;
	// TODO: Implement response setting
	// Set parsed data to HttpResponse object
}

void CgiResponseParser::_parseHeaders(const std::string &headerBlock) {
	(void)headerBlock;
	// TODO: Implement header parsing
	// Parse "Key: Value" format headers
}
