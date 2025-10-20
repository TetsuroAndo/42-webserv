#include "RequestParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Lib/Logger/Log.hpp"

void RequestParserMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {
	if (ctx.parser.isComplete()) {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}

	ctx.parser.parse(ctx.req, ctx.recvBuffer);

	if (ctx.parser.getErrorCode() != 0) {
		ctx.res.statusCode = ctx.parser.getErrorCode();
		ctx.res.headers["Connection"] = "close";
		ctx.res.body = "<html><body><h1>Error</h1></body></html>";
		return;
	}

	if (ctx.parser.isComplete() && proc) {
		proc->next(ctx);
	}
}
