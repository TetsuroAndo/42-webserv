#include "HttpHandlerMiddleware.hpp"
#include "../../../Handler/DeleteHandler.hpp"
#include "../../../Handler/ISubHandler.hpp"
#include "../../../Handler/PostHandler.hpp"
#include "../../../Handler/StaticFileHandler.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include <memory> // for std::auto_ptr

HttpHandlerMiddleware::HttpHandlerMiddleware() {}

void HttpHandlerMiddleware::handle(PipelineContext &ctx,
								   MiddlewareProcessor *next) {
	(void)next;

	std::auto_ptr< ISubHandler > handler;
	const std::string &method = ctx.req.getMethod();

	if (method == "GET" || method == "HEAD") {
		handler.reset(new StaticFileHandler());
	} else if (method == "POST") {
		handler.reset(new PostHandler());
	} else if (method == "DELETE") {
		handler.reset(new DeleteHandler());
	} else {
		// AllowedMethodsMiddlewareで許可されていても、
		// ここで実装されていないメソッド（例: PUT）の場合
		ctx.res.statusCode = HttpStatus::NOT_IMPLEMENTED;
		ctx.res.body = "<html><body><h1>501 Not Implemented</h1></body></html>";
		return;
	}

	if (handler.get()) {
		try {
			ctx.res = handler->handle(ctx);
		} catch (const std::exception &e) {
			ctx.res.statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
			// エラーハンドリング
		}
	}
}
