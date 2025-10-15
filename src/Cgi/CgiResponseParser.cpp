#include "CgiResponseParser.hpp"
#include <sstream>
#include <istream>

CgiResponseParser::CgiResponseParser()
	: _statusCode(200), _statusMessage("OK"), _headers(), _body() {}

CgiResponseParser::~CgiResponseParser() {}

void CgiResponseParser::parse(const std::string &rawResponse) {}

void CgiResponseParser::setResponse(HttpResponse &httpResponse) {}

void CgiResponseParser::_parseHeaders(const std::string &headerBlock) {}


