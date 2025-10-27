#pragma once

#include "../Config/Config.hpp"
#include "../Http/Parser/RequestParser.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "../Socket/Socket.hpp"
#include "HttpConnectionEventHandler.hpp"
#include <string>

class Client; // 前方宣言
class Server; // 前方宣言

/**
 * @class HttpConnection
 * @brief HTTP接続の詳細な処理を担当するクラス。
 * Clientクラスの配下でHTTPリクエストの解析とレスポンス生成を管理する。
 */
class HttpConnection {
public:
	HttpConnection(Client *client, PipelineContext *context,
				   HttpConnectionEventHandler *eventHandler);
	~HttpConnection();

	// HTTP接続の処理
	void processRequest();
	void handleReadEvent();
	void handleWriteEvent();

	// パーサー状態に基づくタイムアウト計算
	time_t calculateTimeout() const;

	// パーサー状態の取得
	RequestParser::ParseState getParserState() const;

private:
	Client *_client;
	PipelineContext *_context;
	HttpConnectionEventHandler *_eventHandler;

	// HTTPリクエストの処理
	void parseRequest();
	void generateResponse();
	void resetForNextRequest();

	HttpConnection(const HttpConnection &);
	HttpConnection &operator=(const HttpConnection &);
};
