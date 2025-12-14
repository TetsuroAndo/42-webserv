#pragma once

#include "../Core/IMiddleware.hpp"

class RequestHeadParserMiddleware : public IMiddleware {
public:
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);
};
