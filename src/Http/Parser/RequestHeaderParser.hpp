#pragma once

#include <string>
#include "../Core/HttpRequest.hpp"
#include "ParseResult.hpp"

class RequestHeaderParser {
public:
	RequestHeaderParser();
	~RequestHeaderParser();
	/**
	 * @brief ヘッダ行を解析し、HttpRequest オブジェクトにヘッダを追加します。
	 * @param request HttpRequest オブジェクトへの参照
	 * @param line 解析するヘッダ行の文字列
	 * @return 解析に成功した場合は 0、失敗した場合はエラーコード
	 */
	ParseResult parse(HttpRequest& request, const std::string &headerBlock, int &errorCode);

private:
	RequestHeaderParser(const RequestHeaderParser&);
	RequestHeaderParser& operator=(const RequestHeaderParser&);

};
