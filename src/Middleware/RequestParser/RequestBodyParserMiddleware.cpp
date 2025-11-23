#include "RequestBodyParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Parser/ParseResult.hpp"
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
		if (proc) {
			proc->next(ctx);
		}
		return;
	default:
		return;
	}
}
