#pragma once

#include <ctime>
#include <string>

#include "../../Config/Config.hpp"
#include "../Core/HttpRequest.hpp"
#include "ParseResult.hpp"

class RequestBodyParser {
public:
	RequestBodyParser(const Config &config);
	~RequestBodyParser();

	void reset();

	/**
	 * @brief HttpRequest
	 * オブジェクトのヘッダ情報に基づき、パーサの初期化を行います。
	 * ヘッダ情報から Content-Length または Transfer-Encoding
	 * に基づく解析モードを設定します。
	 * @param request HttpRequest オブジェクトへの参照
	 * @param errorCode 初期化に失敗した場合に設定されるエラーコード
	 */
	void init(const HttpRequest &request, int &errorCode);

	/**
	 * @brief リクエストボディを解析し、 HttpRequest
	 * オブジェクトにボディデータを格納します。
	 * @param request HttpRequest オブジェクトへの参照
	 * @param buffer 解析するリクエストボディの文字列
	 * @return 消費したバイト数
	 */
	size_t parse(HttpRequest &request, const std::string &buffer,
				 int &errorCode, ParseResult &result);

private:
	enum BodyState {
		UNINITIALIZED,
		IDENTITY,	  // Content-Lengthに基づく受信
		CHUNKED_SIZE, // Chunked: サイズ行の待機
		CHUNKED_DATA, // Chunked: データ部の待機
		CHUNKED_CRLF, // Chunked: 最後のCRLFの待機
		COMPLETE
	};

	BodyState _state;
	size_t _contentLengthRemaining;
	size_t _chunkSize;
	time_t _lastReceiveTime;
	const Config &_config;

	size_t parseIdentity(HttpRequest &request, const std::string &buffer,
						 ParseResult &result);
	size_t parseChunked(HttpRequest &request, const std::string &buffer,
						int &errorCode, ParseResult &result);

	RequestBodyParser(const RequestBodyParser &);
	RequestBodyParser &operator=(const RequestBodyParser &);
};
