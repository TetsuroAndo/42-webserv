#include "HttpHandlerMiddleware.hpp"
#include "../../../Handler/DeleteHandler.hpp"
#include "../../../Handler/ISubHandler.hpp"
#include "../../../Handler/PostHandler.hpp"
#include "../../../Handler/StaticFileHandler.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include "../../../Lib/Logger/Log.hpp"
#include <memory>

HttpHandlerMiddleware::HttpHandlerMiddleware() {}

void HttpHandlerMiddleware::handle(PipelineContext &ctx,
								   MiddlewareProcessor *next) {
	(void)next;

	std::auto_ptr< ISubHandler > handler;
	const std::string &method = ctx.req.getMethod();

	LOG(DEBUG) << "Routing to HTTP handler" << attr("method", method)
			   << attr("path", ctx.req.getPath());

	if (method == "GET" || method == "HEAD") {
		LOG(DEBUG) << "Routing to StaticFileHandler";
		handler.reset(new StaticFileHandler());
	} else if (method == "POST") {
		LOG(DEBUG) << "Routing to PostHandler";
		handler.reset(new PostHandler());
	} else if (method == "DELETE") {
		LOG(DEBUG) << "Routing to DeleteHandler";
		handler.reset(new DeleteHandler());
	} else {
		LOG(WARNING) << "Method is not implemented by HttpHandlerMiddleware"
					 << attr("method", method);
		ctx.res.statusCode = HttpStatus::NOT_IMPLEMENTED;
		ctx.res.body = "<html><body><h1>501 Not Implemented</h1></body></html>";
		return;
	}

	if (handler.get()) {
		try {
			ctx.res = handler->handle(ctx);
		} catch (const std::exception &e) {
			LOG(ERROR) << "HTTP handler (Static/Post/Delete) threw an exception"
					   << attr("error", e.what()) << attr("method", method);
			ctx.res.statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
		}
	}
}
