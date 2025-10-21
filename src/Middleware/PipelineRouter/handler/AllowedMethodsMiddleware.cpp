#include "AllowedMethodsMiddleware.hpp"
#include "../../../Config/Config.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include "../../../Lib/Logger/Log.hpp" // Log.hppを追加

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
	const std::string &path = ctx.req.getPath();
	const Location &loc = ctx.conf.getLocation(path);

	LOG(DEBUG) << "Checking allowed methods" << attr("method", method)
			   << attr("path", path);

	if (loc.allowedMethods.empty() ||
		loc.allowedMethods.find(method) == loc.allowedMethods.end()) {

		std::string allowHeader = getAllowedMethods(loc.allowedMethods);
		LOG(INFO) << "Method not allowed" << attr("method", method)
				  << attr("path", path) << attr("allowed", allowHeader);

		ctx.res.statusCode = HttpStatus::METHOD_NOT_ALLOWED;
		ctx.res.headers["Content-Type"] = "text/html";
		ctx.res.headers["Allow"] = allowHeader;
		ctx.res.body =
			"<html><body><h1>405 Method Not Allowed</h1></body></html>";
		return;
	}

	LOG(DEBUG) << "Method allowed, passing to next middleware"
			   << attr("method", method);
	next->next(ctx);
}
