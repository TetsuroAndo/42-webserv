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
	ParseResult result = parser.parseBody(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE:
		return;
	case PARSE_ERROR:
		ctx.res.setStatusCode(parser.getErrorCode());
		if (proc) {
			ErrorHandlerMiddleware errorHandler(ctx.conf);
			errorHandler.handle(ctx, proc);
		}
		return;
	case PARSE_COMPLETE:
		if (ctx.req.hasHeader("Content-Length") == false) {
			ctx.res.setStatusCode(400);
			if (proc) {
				ErrorHandlerMiddleware errorHandler(ctx.conf);
				errorHandler.handle(ctx, proc);
			}
			return;
		}
		if (ctx.req.getBody().size() !=
			static_cast< size_t >(
				StringOps::stringToInt(ctx.req.getHeader("Content-Length")))) {
			ctx.res.setStatusCode(400);
			if (proc) {
				ErrorHandlerMiddleware errorHandler(ctx.conf);
				errorHandler.handle(ctx, proc);
			}
			return;
		}
		if (proc) {
			proc->next(ctx);
		}
		return;
	default:
		return;
	}
}
