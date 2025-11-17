#pragma once

#include "../Core/HttpRequest.hpp"
#include <string>

class RequestHeadParser {
public:
	RequestHeadParser();
	~RequestHeadParser();

	/**
	 * @brief ヘッダーブロックを解析する
	 * @param request HttpRequest オブジェクトへの参照
	 * @param headerBlock 解析するヘッダーブロックの文字列
	 * @param errorCode エラーコード（失敗時に設定される）
	 * @return 成功した場合 true、失敗した場合 false
	 */
	bool parse(HttpRequest &request, const std::string &headerBlock,
			   int &errorCode);

private:
	RequestHeadParser(const RequestHeadParser &);
	RequestHeadParser &operator=(const RequestHeadParser &);
};
