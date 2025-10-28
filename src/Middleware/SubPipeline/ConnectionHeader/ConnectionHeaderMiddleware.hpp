#pragma once

#include "../../Core/IMiddleware.hpp"

/**
 * @brief Connectionヘッダーを処理するミドルウェア
 *
 * クライアントからのリクエストのConnectionヘッダーに基づいて、
 * レスポンスのConnectionヘッダーを設定する
 */
class ConnectionHeaderMiddleware : public IMiddleware {
public:
	ConnectionHeaderMiddleware();
	~ConnectionHeaderMiddleware();

	void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
	ConnectionHeaderMiddleware(const ConnectionHeaderMiddleware &);
	ConnectionHeaderMiddleware &operator=(const ConnectionHeaderMiddleware &);
};
