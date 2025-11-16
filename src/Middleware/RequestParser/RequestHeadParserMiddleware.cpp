#include "RequestHeadParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Parser/ParseResult.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"
#include <algorithm>

void RequestHeadParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	RequestParser &parser = ctx.parser;

	// 既にヘッダー解析が終わっていればスキップ
	if (parser.getState() >= RequestParser::STATE_BODY) {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}

	// グローバルなボディサイズ制限を設定
	size_t biggestSize = 0;
	if (ctx.conf.hasBiggestMaxRequestBodySize()) {
		biggestSize = ctx.conf.getBiggestMaxRequestBodySize();
	}
	const size_t globalSize = ctx.conf.getMaxRequestBodySize();
	ctx.req.setMaxBodySize(std::max(biggestSize, globalSize));

	ParseResult result = parser.parseHead(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE: {
		return;
	}
	case PARSE_ERROR: {
		ctx.res.setStatusCode(parser.getErrorCode());
		if (proc) {
			ErrorHandlerMiddleware errorHandler(ctx.conf);
			errorHandler.handle(ctx, proc);
		}
		return;
	}
	case PARSE_HEADERS_COMPLETE: {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}
	case PARSE_COMPLETE: {
		// ヘッダー解析中にPARSE_COMPLETEが返ることはない
		parser.setErrorCode(HttpStatus::BAD_REQUEST);
		ctx.res.setStatusCode(parser.getErrorCode());
		if (proc) {
			ErrorHandlerMiddleware errorHandler(ctx.conf);
			errorHandler.handle(ctx, proc);
		}
		return;
	}
	}
}
