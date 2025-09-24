#pragma once

#include "../../Http/Core/HttpRequest.hpp"
#include "../Core/IMiddleware.hpp"
#include <string>
#include <map>

class RequestParserMiddleware : public IMiddleware {
public:
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);
};
