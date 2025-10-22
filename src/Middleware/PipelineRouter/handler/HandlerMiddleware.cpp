#include "HandlerMiddleware.hpp"
#include "../../../Handler/CgiHandler.hpp"
#include "../../../Handler/DeleteHandler.hpp"
#include "../../../Handler/ErrorHandler.hpp"
#include "../../../Handler/PostHandler.hpp"
#include "../../../Handler/StaticFileHandler.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include <memory>

HandlerMiddleware::HandlerMiddleware(CgiManager *cgiManager)
	: _cgiManager(cgiManager) {}

std::string HandlerMiddleware::getAllowedMethods() {
	return "GET, POST, DELETE";
}

void HandlerMiddleware::handle(PipelineContext &ctx,
							   MiddlewareProcessor *next) {
	(void)next;

	const std::string &method = ctx.req.getMethod();
	const Location &loc = ctx.conf.getLocation(ctx.req.getPath());

	if (loc.allowedMethods.empty() ||
		loc.allowedMethods.find(method) == loc.allowedMethods.end()) {
		ctx.res.statusCode = HttpStatus::METHOD_NOT_ALLOWED;
		ctx.res.headers["Content-Type"] = "text/html";

		std::string allowHeader;
		for (std::set< std::string >::const_iterator it =
				 loc.allowedMethods.begin();
			 it != loc.allowedMethods.end(); ++it) {
			if (!allowHeader.empty()) {
				allowHeader += ", ";
			}
			allowHeader += *it;
		}
		ctx.res.headers["Allow"] = allowHeader.empty() ? "" : allowHeader;

		ctx.res.body =
			"<html><body><h1>405 Method Not Allowed</h1></body></html>";
		return;
	}

	std::auto_ptr< ISubHandler > handler;

	bool isCgi = false;
	const std::string &path = ctx.req.getPath();
	for (std::map< std::string, std::string >::const_iterator it =
			 loc.cgiConf.begin();
		 it != loc.cgiConf.end(); ++it) {
		const std::string &ext = it->first;
		if (path.size() >= ext.size() &&
			path.compare(path.size() - ext.size(), ext.size(), ext) == 0) {
			isCgi = true;
			break;
		}
	}

	if (method == "GET" || method == "HEAD") {
		if (isCgi) {
			handler.reset(new CgiHandler(_cgiManager));
		} else {
			handler.reset(new StaticFileHandler());
		}
	} else if (method == "POST") {
		if (isCgi) {
			handler.reset(new CgiHandler(_cgiManager));
		} else {
			handler.reset(new PostHandler());
		}
	} else if (method == "DELETE") {
		handler.reset(new DeleteHandler());
	} else {
		ctx.res.statusCode = HttpStatus::METHOD_NOT_ALLOWED;
		ctx.res.headers["Content-Type"] = "text/html";
		ctx.res.headers["Allow"] = getAllowedMethods();
		ctx.res.body =
			"<html><body><h1>405 Method Not Allowed</h1></body></html>";
		return;
	}

	if (handler.get()) {
		try {
			ctx.res = handler->handle(ctx);
		} catch (const std::exception &e) {
			ctx.res.statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
		}
	}
}
