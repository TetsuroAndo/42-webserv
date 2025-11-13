#pragma once

#include "../../Http/Core/HttpRequest.hpp"
#include "../Core/IMiddleware.hpp"
#include <map>
#include <string>

class RequestHeadParserMiddleware : public IMiddleware {
public:
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);
};
