#pragma once

#include "../../Core/IMiddleware.hpp"

class HttpHandlerMiddleware : public IMiddleware {
public:
	HttpHandlerMiddleware();
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *next);

private:
	HttpHandlerMiddleware(const HttpHandlerMiddleware &);
	HttpHandlerMiddleware &operator=(const HttpHandlerMiddleware &);
};
