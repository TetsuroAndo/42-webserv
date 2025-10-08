#pragma once

#include <string>
#include <map>
#include "../Http/Core/HttpResponse.hpp" // HttpResponseクラスのパスを仮定

class CgiResponseParser {
public:
	CgiResponseParser();
	~CgiResponseParser();

	void parse(const std::string &rawResponse);
	void setResponse(HttpResponse &httpResponse);

private:
	std::string _cgiHeadersStr;
	std::string _cgiBodyStr;
	int _statusCode;
	std::string _statusMessage;
	std::map<std::string, std::string> _headers;

	void _parseHeaders();

	CgiResponseParser(const CgiResponseParser&);
	CgiResponseParser &operator=(const CgiResponseParser&);
};
