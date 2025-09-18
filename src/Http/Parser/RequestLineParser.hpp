#pragma once

#include "../Core/HttpRequest.hpp"
#include "ParseResult.hpp"
#include <string>

class RequestLineParser {
public:
	RequestLineParser();
	~RequestLineParser();

	/**
	 * @brief リクエストラインを解析し、メソッド、パス、バージョンを HttpRequest オブジェクトに設定します。
	 * @param request HttpRequest オブジェクトへの参照
	 * @param line 解析するリクエストラインの文字列
	 * @return 解析に成功した場合は 0、失敗した場合はエラーコード
	 */
	ParseResult parse(HttpRequest& request, const std::string& line, int &errorCode);

private:
	void parseQuery(HttpRequest& request, const std::string& queryString);
	std::string parsePath(HttpRequest& request, const std::string& uri);

	RequestLineParser(const RequestLineParser&);
	RequestLineParser& operator=(const RequestLineParser&);
};
