#include "HandlerMiddleware.hpp"
#include "../../../Handler/ISubHandler.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include "../../../Lib/Logger/Log.hpp"

#include <map>
#include <sstream>

std::string HandlerMiddleware::getAllowedMethods() {
	const char *order[] = {"GET", "HEAD", "POST", "PUT", "DELETE"};
	const size_t orderSize = sizeof(order) / sizeof(order[0]);

	std::stringstream ss;
	bool first = true;

	for (size_t i = 0; i < orderSize; ++i) {
		if (_handlers.find(order[i]) != _handlers.end()) {
			if (!first) {
				ss << ", ";
			}
			ss << order[i];
			first = false;
		}
	}
	return ss.str();
}

HandlerMiddleware::HandlerMiddleware(
	const std::map< std::string, ISubHandler * > &handlers)
	: _handlers(handlers) {}

HandlerMiddleware::~HandlerMiddleware() {
	for (std::map< std::string, ISubHandler * >::iterator it =
			 _handlers.begin();
		 it != _handlers.end(); ++it) {
		delete it->second;
	}
	_handlers.clear();
}

void HandlerMiddleware::handle(PipelineContext &ctx,
							   MiddlewareProcessor *proc) {
	(void)proc;
	const std::string &method = ctx.req.getMethod();

	const std::map< std::string, ISubHandler * >::const_iterator it =
		_handlers.find(method);

	if (it != _handlers.end()) {
		ISubHandler *handler = it->second;
		if (handler == NULL) {
			ctx.setError(HttpStatus::INTERNAL_SERVER_ERROR);
			return;
		}
		try {
			ctx.res = handler->handle(ctx);
		} catch (const std::exception &e) {
			LOG(ERROR) << "Handler exception: " << e.what();
			ctx.setError(HttpStatus::INTERNAL_SERVER_ERROR);
		}
	} else {
		// 対応するハンドラがない場合
		ctx.setError(HttpStatus::METHOD_NOT_ALLOWED);
		ctx.res.setHeader("Allow", getAllowedMethods());
	}
}
