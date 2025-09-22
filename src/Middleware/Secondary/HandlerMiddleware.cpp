#include "HandlerMiddleware.hpp"
#include "../../Handler/CgiHandler.hpp"
#include "../../Handler/DeleteHandler.hpp"
#include "../../Handler/StaticFileHandler.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include <sstream>

HandlerMiddleware::HandlerMiddleware() {
	_staticFileHandler = new StaticFileHandler();
	_deleteHandler = new DeleteHandler();
	// _cgiHandler = new CgiHandler();
}

HandlerMiddleware::~HandlerMiddleware() {
	delete _staticFileHandler;
	delete _deleteHandler;
	// delete _cgiHandler;
}

void HandlerMiddleware::handle(PipelineContext &ctx, MiddlewareProcessor *proc) {
	(void)proc; // This middleware is the end of the line

	ISubHandler *handler = NULL;
	const std::string &method = ctx.req->getMethod();

	// TODO: Add more sophisticated routing (e.g., based on path for CGI)
	if (method == "GET" || method == "HEAD") {
		handler = _staticFileHandler;
	} else if (method == "DELETE") {
		handler = _deleteHandler;
	} else if (method == "POST") {
		// Placeholder for CGI or Upload handler
		// handler = _cgiHandler;
	}

	if (handler) {
		try {
			*ctx.res = handler->handle(*ctx.req, ctx.conf);
		} catch (const std::exception &e) {
			ctx.res->setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
			ctx.res->setHeader("Content-Type", "text/html");
			std::ostringstream oss;
			oss << "<html><body><h1>500 Internal Server Error</h1><p>"
				<< e.what() << "</p></body></html>";
			ctx.res->setBody(oss.str());
		}
	} else {
		ctx.res->setStatusCode(HttpStatus::NOT_IMPLEMENTED);
		ctx.res->setHeader("Content-Type", "text/html");
		std::ostringstream oss;
		oss << "<html><body><h1>501 Not Implemented</h1></body></html>";
		ctx.res->setBody(oss.str());
	}
}
