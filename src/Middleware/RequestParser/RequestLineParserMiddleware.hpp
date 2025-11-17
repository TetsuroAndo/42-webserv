#pragma once

#include "../Core/IMiddleware.hpp"

class RequestLineParserMiddleware : public IMiddleware {
public:
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);
};
