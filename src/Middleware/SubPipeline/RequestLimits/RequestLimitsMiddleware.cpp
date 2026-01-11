#include "RequestLimitsMiddleware.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include "../../../Lib/StringOps/StringOps.hpp"
#include "../ErrorHandler/ErrorHandlerMiddleware.hpp"

RequestLimitsMiddleware::RequestLimitsMiddleware() {}

RequestLimitsMiddleware::~RequestLimitsMiddleware() {}

void RequestLimitsMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {
	const RequestParser::ParseState state = ctx.parser.getState();
	if (state == RequestParser::STATE_REQUEST_LINE ||
		state == RequestParser::STATE_HEADERS) {
		return;
	}

	const Config &c = *ctx.conf;
	const Location &loc = c.getLocation(ctx.req.getPath());

	// Check: Request Header Size
	const size_t headerBytes = ctx.parser.getLastHeaderBytes();
	if (&c != NULL && c.getMaxRequestHeaderSize() < headerBytes) {
		ctx.setError(HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE);
		if (proc) {
			ErrorHandlerMiddleware errorHandler;
			errorHandler.handle(ctx, proc);
		}
		return;
	}

	// Set: Request Body Size limit
	size_t maxBodySize = c.getMaxRequestBodySize();
	if (loc.hasMaxRequestBodySize) {
		maxBodySize = loc.maxRequestBodySize;
	}
	ctx.req.setMaxBodySize(maxBodySize);
	ctx.parser.getBodyParser().setChunkedTimeoutSec(loc.chunkedTimeoutSec);

	// Check: Content-Length Size
	if (ctx.req.hasHeader("Content-Length")) {
		size_t contentLength = 0;
		if (!StringOps::decStrToSize(ctx.req.getHeader("Content-Length"),
									 contentLength)) {
			ctx.setError(HttpStatus::BAD_REQUEST);
			if (proc) {
				ErrorHandlerMiddleware errorHandler;
				errorHandler.handle(ctx, proc);
			}
			return;
		}
		if (maxBodySize < contentLength) {
			ctx.setError(HttpStatus::PAYLOAD_TOO_LARGE);
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
