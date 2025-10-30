#include "ConnectionHeaderMiddleware.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include "../../../Lib/Logger/Log.hpp"
#include <sstream>

ConnectionHeaderMiddleware::ConnectionHeaderMiddleware() {}

ConnectionHeaderMiddleware::~ConnectionHeaderMiddleware() {}

void ConnectionHeaderMiddleware::handle(PipelineContext &ctx,
										MiddlewareProcessor *proc) {
	const HttpRequest &req = ctx.req;
	HttpResponse &res = ctx.res;

	// クライアントのリクエストからConnectionヘッダーを取得
	const std::string &connection = req.getHeader("connection");

	// クライアントがConnection: closeを要求している場合
	if (connection == "close") {
		// レスポンスでConnection: closeを返す
		res.setHeader("Connection", "close");
	} else {
		// デフォルトでkeep-aliveを許可
		// HTTP/1.1の場合は、デフォルトでkeep-aliveなので明示的にヘッダーを設定しない
		// HTTP/1.0の場合はConnection: closeを設定
		if (req.getVersion() == "HTTP/1.0") {
			res.setHeader("Connection", "close");
		}
	}

	// 次のミドルウェアに進む
	if (proc) {
		proc->next(ctx);
	}
}
