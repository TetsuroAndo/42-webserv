#include "RequestParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

#include "../../Lib/Logger/Log.hpp"
#include <algorithm>

void RequestParserMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {
	if (ctx.parser.getState() == RequestParser::STATE_REQUEST_LINE) {
		unsigned int biggestSize = 0;
		if (ctx.conf.hasBiggestMaxRequestBodySize()) {
			biggestSize = ctx.conf.getBiggestMaxRequestBodySize();
		}
		const unsigned int globalSize = ctx.conf.getMaxRequestBodySize();
		ctx.req.setMaxBodySize(std::max(biggestSize, globalSize));
	}

	ParseResult result = PARSE_HEADERS_COMPLETE;
	while (PARSE_HEADERS_COMPLETE == result) {
		result = ctx.parser.parse(ctx.req, ctx.recvBuffer);
		if (PARSE_HEADERS_COMPLETE == result) {
			const Location loc = ctx.conf.getLocation(ctx.req.getPath());
			if (loc.maxRequestBodySize != -1) {
				const std::size_t effectiveLimit =
					static_cast< std::size_t >(loc.maxRequestBodySize);
				if (ctx.req.getBody().size() > effectiveLimit) {
					ctx.req.setMaxBodySize(effectiveLimit);
				}
			}
		}
	}

	switch (result) {
	case PARSE_INCOMPLETE: {
		return;
	}
	case PARSE_ERROR: {
		const int code = ctx.parser.getErrorCode();
		ctx.res.setStatusCode(code);
		if (proc) {
			ErrorHandlerMiddleware errorHandler(ctx.conf);
			errorHandler.handle(ctx, proc);
		}
		return;
	}
	case PARSE_HEADERS_COMPLETE: {
		// 呼ばれない
		return;
	}
	case PARSE_COMPLETE: {
		if (proc) {
			proc->next(ctx);
		}
	}
	}
}
