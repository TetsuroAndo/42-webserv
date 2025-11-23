#include "RequestHeadParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Parser/ParseResult.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"
#include <algorithm>

void RequestHeadParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	RequestParser &parser = ctx.parser;

	if (parser.getState() != RequestParser::STATE_HEADERS) {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}

	const std::string &requestPath = ctx.req.getPath();
	LOG(DEBUG) << "RequestHeadParserMiddleware: Parsing headers"
			   << attr("path", requestPath)
			   << attr("method", ctx.req.getMethod())
			   << attr("path_empty", requestPath.empty());
	const Location loc = ctx.conf.getLocation(requestPath);
	size_t maxBodySize = ctx.conf.getMaxRequestBodySize();
	if (loc.hasMaxRequestBodySize) {
		maxBodySize = loc.maxRequestBodySize;
	}
	ctx.req.setMaxBodySize(maxBodySize);

	ParseResult result = parser.parseHeaders(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE:
		return; // データが足りない（\r\n\r\nがまだない）
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
