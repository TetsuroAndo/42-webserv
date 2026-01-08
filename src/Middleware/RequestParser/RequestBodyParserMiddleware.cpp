#include "RequestBodyParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Parser/ParseResult.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

void RequestBodyParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	RequestParser &parser = ctx.parser;

	if (parser.getState() != RequestParser::STATE_BODY) {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}

	// ボディのパーシングを行う
	const ParseResult result = parser.parseBody(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE:
		return;
	case PARSE_ERROR:
		ctx.setError(parser.getErrorCode());
		if (proc) {
			ErrorHandlerMiddleware errorHandler;
			errorHandler.handle(ctx, proc);
		}
		return;
	case PARSE_COMPLETE:
		if (ctx.req.getBody().size() > ctx.req.getMaxBodySize()) {
			ctx.setError(HttpStatus::PAYLOAD_TOO_LARGE);
			if (proc) {
				ErrorHandlerMiddleware errorHandler;
				errorHandler.handle(ctx, proc);
			}
			return;
		}
		if (ctx.req.hasHeader("Content-Length") == false &&
			ctx.recvBuffer.size() != 0) {
			ctx.setError(HttpStatus::BAD_REQUEST);
			if (proc) {
				ErrorHandlerMiddleware errorHandler;
				errorHandler.handle(ctx, proc);
			}
			return;
		}
		if (ctx.req.hasHeader("Content-Length") == true) {
			if (!ctx.req.hasHeader("Transfer-Encoding") &&
				!ctx.recvBuffer.empty()) {
				ctx.setError(HttpStatus::BAD_REQUEST);
				if (proc) {
					ErrorHandlerMiddleware errorHandler;
					errorHandler.handle(ctx, proc);
				}
				return;
			}
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
			if (ctx.req.getBody().size() != contentLength) {
				ctx.setError(HttpStatus::BAD_REQUEST);
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
		return;
	default:
		return;
	}
}
