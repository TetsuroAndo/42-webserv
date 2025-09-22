#pragma once

#include "../../Http/Core/HttpRequest.hpp"
#include "../../Config/Config.hpp"
#include "PipelineContext.hpp"
#include <string>
#include <vector>

class IMiddleware;

class MiddlewareProcessor {
public:
	MiddlewareProcessor();
	~MiddlewareProcessor();

	void addMiddleware(IMiddleware *middleware);
	void handle(PipelineContext &ctx);
	void next(PipelineContext &ctx);

private:
	std::vector<IMiddleware*> _middlewares;
	size_t _currentIndex;
};
