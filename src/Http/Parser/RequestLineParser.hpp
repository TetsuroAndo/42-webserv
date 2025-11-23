#pragma once

#include "../Core/HttpRequest.hpp"
#include <string>

class RequestLineParser {
public:
	RequestLineParser();
	~RequestLineParser();

	/**
	 * @brief リクエストラインを解析する
	 * @param request HttpRequest オブジェクトへの参照
	 * @param line 解析するリクエストラインの文字列
	 * @param errorCode エラーコード（失敗時に設定される）
	 * @return 成功した場合 true、失敗した場合 false
	 */
	bool parse(HttpRequest &request, const std::string &line, int &errorCode);

private:
	RequestLineParser(const RequestLineParser &);
	RequestLineParser &operator=(const RequestLineParser &);
};
