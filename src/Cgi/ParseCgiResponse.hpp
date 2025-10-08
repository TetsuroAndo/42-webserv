#pragma once

#include <string>
#include <map>
#include "../Http/Core/HttpResponse.hpp" // HttpResponseクラスのパスを仮定

/*
 * CgiResponseParserクラス
 *
 * CGIスクリプトからの生の出力文字列をパースし、
 * ステータス、ヘッダ、ボディに分割する責務を持つ。
 */
class CgiResponseParser {
public:
	CgiResponseParser();
	~CgiResponseParser();

	void parse(const std::string &rawResponse);
	void setResponse(HttpResponse &httpResponse);

private:
	CgiResponseParser(const CgiResponseParser &other);
	CgiResponseParser &operator=(const CgiResponseParser &other);

	std::string _cgiHeadersStr;
	std::string _cgiBodyStr;
	int _statusCode;
	std::string _statusMessage;
	std::map<std::string, std::string> _headers;

	void _parseHeaders();
};
