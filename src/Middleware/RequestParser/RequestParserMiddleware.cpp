#include "RequestParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

#include "../../Lib/Logger/Log.hpp"
#include <sstream>

void RequestParserMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {
	ctx.req.setMaxBodySize(
		std::max(ctx.conf.getBiggestMaxRequestBodySize(),
				 static_cast< int >(ctx.req.getMaxBodySize())));
	const ParseResult result = ctx.parser.parse(ctx.req, ctx.recvBuffer);

	if (result == PARSE_COMPLETE) {
		const Location loc = ctx.conf.getLocation(ctx.req.getPath());
		// このリクエストに適用されるべき「有効な」リミットを決定する
		std::size_t effectiveLimit;
		if (loc.maxRequestBodySize == -1) {
			effectiveLimit = ctx.conf.getMaxRequestBodySize();
		} else {
			effectiveLimit = static_cast< std::size_t >(loc.maxRequestBodySize);
		}
		// 有効なリミットとボディサイズを比較する
		if (ctx.req.getBody().size() > effectiveLimit) {
			ctx.res.setStatusCode(HttpStatus::PAYLOAD_TOO_LARGE);
			if (proc) {
				ErrorHandlerMiddleware errorHandler(ctx.conf);
				errorHandler.handle(ctx, proc);
			}
			return;
		}
		if (proc) {
			proc->next(ctx);
		}
	} else if (result == PARSE_ERROR) {
		const int code = ctx.parser.getErrorCode();
		ctx.res.setStatusCode(code);
		if (proc) {
			ErrorHandlerMiddleware errorHandler(ctx.conf);
			errorHandler.handle(ctx, proc);
		}
	}
}
