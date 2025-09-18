#pragma once

#include <string>
#include "../Core/HttpRequest.hpp"
#include "ParseResult.hpp"

class RequestBodyParser {
public:
	RequestBodyParser();
	~RequestBodyParser();

	void reset();

	/**
	 * @brief HttpRequest オブジェクトのヘッダ情報に基づき、パーサの初期化を行います。
	 * ヘッダ情報から Content-Length または Transfer-Encoding に基づく解析モードを設定します。
	 * @param request HttpRequest オブジェクトへの参照
	 * @param errorCode 初期化に失敗した場合に設定されるエラーコード
	 */
	void init(const HttpRequest& request, int& errorCode);

	/**
	 * @brief リクエストボディを解析し、 HttpRequest オブジェクトにボディデータを格納します。
	 * @param request HttpRequest オブジェクトへの参照
	 * @param buffer 解析するリクエストボディの文字列
	 * @return 解析に成功した場合は 0、失敗した場合はエラーコード
	 */
	ParseResult parse(HttpRequest& request, std::string& buffer, int& errorCode);

private:
	enum BodyState {
		UNINITIALIZED,
		IDENTITY,          // Content-Lengthに基づく受信
		CHUNKED_SIZE,      // Chunked: サイズ行の待機
		CHUNKED_DATA,      // Chunked: データ部の待機
		CHUNKED_CRLF,   // Chunked: 最後のCRLFの待機
		COMPLETE
	};

	BodyState _state;
	size_t _contentLengthRemaining;
	size_t _chunkSize;

	ParseResult parseIdentity(HttpRequest& request, std::string& buffer, int &errorCode);
	ParseResult parseChunked(HttpRequest& request, std::string& buffer, int& errorCode);

	RequestBodyParser(const RequestBodyParser&);
	RequestBodyParser& operator=(const RequestBodyParser&);

};
