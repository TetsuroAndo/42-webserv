#pragma once

#include "../Config/Config.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "MiddlewareProcessor.hpp"
#include "PipelineContext.hpp"

class IMiddleware {
public:
	virtual ~IMiddleware() {}
	virtual void handle(PipelineContext& ctx, MiddlewareProcessor& proc) = 0;
};
