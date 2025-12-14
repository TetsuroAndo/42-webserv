#pragma once

#include "MiddlewareProcessor.hpp"
#include "PipelineContext.hpp"

class IMiddleware {
public:
	virtual ~IMiddleware() {}

	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc) = 0;
};
