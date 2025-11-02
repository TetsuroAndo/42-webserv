#include "RequestParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

#include "../../Lib/Logger/Log.hpp"
#include <sstream>

void RequestParserMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {
	const ParseResult result = ctx.parser.parse(ctx.req, ctx.recvBuffer);

	if (result == PARSE_COMPLETE) {
		const Location loc = ctx.conf.getLocation(ctx.req.getPath());
		if (-1 < loc.maxRequestBodySize) {
			std::size_t maxRequestBodySize = loc.maxRequestBodySize;
			if (maxRequestBodySize < ctx.req.getBody().size()) {
				ctx.res.setStatusCode(HttpStatus::PAYLOAD_TOO_LARGE);
				if (proc) {
					ErrorHandlerMiddleware errorHandler(ctx.conf);
					errorHandler.handle(ctx, proc);
				}
				return;
			}
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
