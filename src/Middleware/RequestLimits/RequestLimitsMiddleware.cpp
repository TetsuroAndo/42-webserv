#include "RequestLimitsMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

RequestLimitsMiddleware::RequestLimitsMiddleware() {}

RequestLimitsMiddleware::~RequestLimitsMiddleware() {}

void RequestLimitsMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {
	const RequestParser::ParseState state = ctx.parser.getState();
	if (state == RequestParser::STATE_REQUEST_LINE ||
		state == RequestParser::STATE_HEADERS) {
		return;
	}

	const Config &config = *ctx.conf;
	const Location &loc = config.getLocation(ctx.req.getPath());
	size_t maxBodySize = config.getMaxRequestBodySize();
	if (loc.hasMaxRequestBodySize) {
		maxBodySize = loc.maxRequestBodySize;
	}
	ctx.req.setMaxBodySize(maxBodySize);
	ctx.parser.getBodyParser().setChunkedTimeoutSec(loc.chunkedTimeoutSec);

	if (ctx.req.hasHeader("Content-Length")) {
		size_t contentLength = 0;
		if (!StringOps::decStrToSize(ctx.req.getHeader("Content-Length"),
									 contentLength)) {
			ctx.res.setStatusCode(HttpStatus::BAD_REQUEST);
			ctx.parser.setErrorCode(HttpStatus::BAD_REQUEST);
			if (proc) {
				ErrorHandlerMiddleware errorHandler;
				errorHandler.handle(ctx, proc);
			}
			return;
		}
		if (maxBodySize < contentLength) {
			ctx.res.setStatusCode(HttpStatus::PAYLOAD_TOO_LARGE);
			ctx.parser.setErrorCode(HttpStatus::PAYLOAD_TOO_LARGE);
			if (proc) {
				ErrorHandlerMiddleware errorHandler;
				errorHandler.handle(ctx, proc);
			}
			return;
		}
	}

	if (proc) {
		proc->next(ctx);
	}
}
