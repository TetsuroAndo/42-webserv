#pragma once

#include "../../Config/Config.hpp"
#include "../Core/IMiddleware.hpp"
#include "../Core/MiddlewareProcessor.hpp"
#include <string>

class SessionMiddleware : public IMiddleware {
public:
	SessionMiddleware();
	virtual ~SessionMiddleware();

	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
};
