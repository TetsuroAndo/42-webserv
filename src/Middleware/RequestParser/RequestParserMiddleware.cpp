#include "RequestParserMiddleware.hpp"
#include "../Primary/RequestParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include <sstream>

void RequestParserMiddleware::handle(PipelineContext &ctx,
									 MiddlewareProcessor *proc) {
	if (!ctx.req || !ctx.res) {
		return;
	}

	const ParseResult result = ctx.parser.parse(*ctx.req, ctx.recvBuffer);

	if (result == PARSE_COMPLETE) {
		if (proc) {
			proc->next(ctx);
		}
	} else if (result == PARSE_ERROR) {
		const int code = ctx.parser.getErrorCode();
		ctx.res->setStatusCode(code);
		ctx.res->setHeader("Content-Type", "text/html");
		const std::string &reason = HttpStatus::getReason(code);
		std::ostringstream oss;
		oss << "<html><head><title>" << code << " " << reason
			<< "</title></head>"
			<< "<body><h1>" << code << " " << reason << "</h1></body></html>";
		ctx.res->setBody(oss.str());
	}
}
