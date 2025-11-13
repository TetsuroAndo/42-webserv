#include "RequestHeadParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Lib/Logger/Log.hpp"
#include <algorithm>

void RequestHeadParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	if (ctx.parser.getState() == RequestParser::STATE_REQUEST_LINE) {
		unsigned int biggestSize = 0;
		if (ctx.conf.hasBiggestMaxRequestBodySize()) {
			biggestSize = ctx.conf.getBiggestMaxRequestBodySize();
		}
		const unsigned int globalSize = ctx.conf.getMaxRequestBodySize();
		ctx.req.setMaxBodySize(std::max(biggestSize, globalSize));
	}

	// ヘッダーのパーシングのみを行う
	ParseResult result = ctx.parser.parse(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE: {
		return;
	}
	case PARSE_ERROR: {
		ctx.res.setStatusCode(ctx.parser.getErrorCode());
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
		// ヘッダーパーシング中に完了することはないが、念のため
		if (proc) {
			proc->next(ctx);
		}
		return;
	}
	}
}
