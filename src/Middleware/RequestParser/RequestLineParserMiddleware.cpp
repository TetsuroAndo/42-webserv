#include "RequestLineParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Parser/ParseResult.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

void RequestLineParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	RequestParser &parser = ctx.parser;

	// 自分の担当する状態でなければ、次のミドルウェアに処理を渡す
	if (parser.getState() != RequestParser::STATE_REQUEST_LINE) {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}

	ParseResult result = parser.parseRequestLine(ctx.req, ctx.recvBuffer);

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
