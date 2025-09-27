#include "HandlerMiddleware.hpp"
#include "../../Handler/ISubHandler.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include <sstream>

HandlerMiddleware::HandlerMiddleware(
	const std::map<std::string, ISubHandler *> &handlers)
	: _handlers(handlers) {}

HandlerMiddleware::~HandlerMiddleware() {
	for (std::map<std::string, ISubHandler *>::iterator it = _handlers.begin();
		 it != _handlers.end(); ++it) {
		delete it->second;
	}
	_handlers.clear();
}

void HandlerMiddleware::handle(PipelineContext &ctx,
							   MiddlewareProcessor *proc) {
	(void)proc;
	const std::string &method = ctx.req->getMethod();

	const std::map<std::string, ISubHandler *>::const_iterator it =
		_handlers.find(method);

	if (it != _handlers.end()) {
		ISubHandler *handler = it->second;
		try {
			*ctx.res = handler->handle(*ctx.req, ctx.conf);
		} catch (...) {
			ctx.res->setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
			ctx.res->setHeader("Content-Type", "text/html");
			ctx.res->setBody(
				"<html><body><h1>500 Internal Server Error</h1></body></html>");
		}
	} else {
		// 対応するハンドラがない場合
		ctx.res->setStatusCode(HttpStatus::METHOD_NOT_ALLOWED);
		ctx.res->setHeader("Content-Type", "text/html");
		ctx.res->setBody(
			"<html><body><h1>405 Method Not Allowed</h1></body></html>");
	}
}
