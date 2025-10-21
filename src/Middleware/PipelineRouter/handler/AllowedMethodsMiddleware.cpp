#include "AllowedMethodsMiddleware.hpp"
#include "../../../Config/Config.hpp"
#include "../../../Http/Core/HttpStatus.hpp"

AllowedMethodsMiddleware::AllowedMethodsMiddleware() {}

std::string AllowedMethodsMiddleware::getAllowedMethods(
	const std::set< std::string > &allowedMethods) {
	std::string allowHeader;
	for (std::set< std::string >::const_iterator it = allowedMethods.begin();
		 it != allowedMethods.end(); ++it) {
		if (!allowHeader.empty()) {
			allowHeader += ", ";
		}
		allowHeader += *it;
	}
	return allowHeader.empty() ? "" : allowHeader;
}

void AllowedMethodsMiddleware::handle(PipelineContext &ctx,
									  MiddlewareProcessor *next) {
	const std::string &method = ctx.req.getMethod();
	const Location &loc = ctx.conf.getLocation(ctx.req.getPath());

	if (loc.allowedMethods.empty() ||
		loc.allowedMethods.find(method) == loc.allowedMethods.end()) {

		// メソッドが許可されていない -> 405を返して終了
		ctx.res.statusCode = HttpStatus::METHOD_NOT_ALLOWED;
		ctx.res.headers["Content-Type"] = "text/html";
		ctx.res.headers["Allow"] = getAllowedMethods(loc.allowedMethods);
		ctx.res.body =
			"<html><body><h1>405 Method Not Allowed</h1></body></html>";
		return; // パイプラインをここで停止
	}

	// メソッドが許可されている -> 次のミドルウェアへ
	next->next(ctx);
}
