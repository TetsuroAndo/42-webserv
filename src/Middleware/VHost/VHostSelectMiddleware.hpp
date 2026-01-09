#pragma once

#include "../Core/IMiddleware.hpp"

class VHostSelectMiddleware : public IMiddleware {
public:
	VHostSelectMiddleware();
	~VHostSelectMiddleware();

	void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
	VHostSelectMiddleware(const VHostSelectMiddleware &);
	VHostSelectMiddleware &operator=(const VHostSelectMiddleware &);
};
