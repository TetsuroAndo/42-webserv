#include "CgiResponseParser.hpp"
#include <istream>
#include <sstream>

CgiResponseParser::CgiResponseParser()
	: _statusCode(200), _statusMessage("OK"), _headers(), _body() {}

CgiResponseParser::~CgiResponseParser() {}

void CgiResponseParser::parse(const std::string &rawResponse) {
	(void)rawResponse;
}

void CgiResponseParser::setResponse(HttpResponse &httpResponse) {
	(void)httpResponse;
}

void CgiResponseParser::_parseHeaders(const std::string &headerBlock) {
	(void)headerBlock;
}
