#pragma once

#include <string>
#include "../Core/HttpRequest.hpp"
#include "RequestParser.hpp"
#include "ParseResult.hpp"

class RequestBodyParser {
public:
	RequestBodyParser();
	~RequestBodyParser();

	/**
	 * @brief リクエストボディを解析し、HttpRequest オブジェクトに設定します。
	 * @param request HttpRequest オブジェクトへの参照
	 * @param buffer 解析するリクエストボディの文字列
	 * @return 解析に成功した場合は 0、失敗した場合はエラーコード
	 */
	ParseResult parse(HttpRequest& request, std::string& buffer, int& errorCode);

private:
	enum ParseState {
		STATE_INIT,
		STATE_CONTENT_LENGTH,
		STATE_CHUNKED_SIZE,
		STATE_CHUNKED_DATA,
		STATE_COMPLETE,
		STATE_ERROR
	};

	ParseState _state;
	size_t _bodySizeRemaining;
	size_t _chunkSize;

	void init(const HttpRequest& request, int& errorCode);
	ParseResult parseContentLength(HttpRequest& request, std::string& buffer);
	ParseResult parseChunked(HttpRequest& request, std::string& buffer, int& errorCode);

	RequestBodyParser(const RequestBodyParser&);
	RequestBodyParser& operator=(const RequestBodyParser&);

};
