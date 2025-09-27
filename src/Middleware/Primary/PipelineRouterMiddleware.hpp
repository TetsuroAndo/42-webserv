#pragma once

#include "../Core/IMiddleware.hpp"
#include <map>
#include <string>

typedef std::map<std::string, class MiddlewareProcessor *> RouteMap;

class PipelineRouterMiddleware : public IMiddleware {
public:
	PipelineRouterMiddleware(const RouteMap &routes);
	~PipelineRouterMiddleware();
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
	RouteMap _routes;

	PipelineRouterMiddleware(const PipelineRouterMiddleware &);
	PipelineRouterMiddleware &operator=(const PipelineRouterMiddleware &);
};
