#include "RequestLineParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Parser/ParseResult.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

void RequestLineParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	RequestParser &parser = ctx.parser;

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
		ctx.setError(parser.getErrorCode());
		if (proc) {
			ErrorHandlerMiddleware errorHandler;
			errorHandler.handle(ctx, proc);
		}
		return;
	case PARSE_COMPLETE:
		LOG(DEBUG) << "RequestLineParserMiddleware: Parsed request line"
				   << attr("method", ctx.req.getMethod())
				   << attr("path", ctx.req.getPath())
				   << attr("version", ctx.req.getVersion());
		if (proc) {
			proc->next(ctx);
		}
		return;
	default:
		return;
	}
}
