#pragma once

#include "../../Core/IMiddleware.hpp"

class RequestLimitsMiddleware : public IMiddleware {
public:
	RequestLimitsMiddleware();
	~RequestLimitsMiddleware();

	void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
	RequestLimitsMiddleware(const RequestLimitsMiddleware &);
	RequestLimitsMiddleware &operator=(const RequestLimitsMiddleware &);
};
