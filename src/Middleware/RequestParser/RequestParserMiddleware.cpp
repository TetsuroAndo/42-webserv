#include "RequestParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

#include "../../Lib/Logger/Log.hpp"
#include <algorithm>

void RequestParserMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {

	// パーサーが初期状態の場合、まず「緩い」上限を設定する
	// これにより、ヘッダーパース中にバッファが大きくなりすぎても
	// _bodyParserが（もし動作しても）誤ってエラーを出すのを防ぐ
	if (ctx.parser.getState() == RequestParser::STATE_REQUEST_LINE) {
		unsigned int biggestSize = 0;
		if (ctx.conf.hasBiggestMaxRequestBodySize()) {
			biggestSize = ctx.conf.getBiggestMaxRequestBodySize();
		}
		unsigned int globalSize = ctx.conf.getMaxRequestBodySize();
		ctx.req.setMaxBodySize(std::max(biggestSize, globalSize));
	}

	const ParseResult result = ctx.parser.parse(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE:
		return;

	case PARSE_ERROR: {
		const int code = ctx.parser.getErrorCode();
		ctx.res.setStatusCode(code);
		if (proc) {
			ErrorHandlerMiddleware errorHandler(ctx.conf);
			errorHandler.handle(ctx, proc);
		}
		return;
	}

	case PARSE_HEADERS_COMPLETE: {
		const Location loc = ctx.conf.getLocation(ctx.req.getPath());
		// このリクエストに適用されるべき「有効な」リミットを決定する
		std::size_t effectiveLimit;
		if (loc.maxRequestBodySize == -1) {
			effectiveLimit = ctx.conf.getMaxRequestBodySize();
		} else {
			effectiveLimit = static_cast< std::size_t >(loc.maxRequestBodySize);
		}
		ctx.req.setMaxBodySize(effectiveLimit);
		return;
	}

	case PARSE_COMPLETE: {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}
	}
}
