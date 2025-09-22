#pragma once

#include "../Core/IMiddleware.hpp"
#include <string>
#include <map>

typedef std::map<std::string, class MiddlewareProcessor*> RouteMap;

class PipelineRouterMiddleware : public IMiddleware {
public:
	PipelineRouterMiddleware(const RouteMap &routes);
	~PipelineRouterMiddleware();

private:
	RouteMap _routes;

	PipelineRouterMiddleware(const PipelineRouterMiddleware&);
	PipelineRouterMiddleware& operator=(const PipelineRouterMiddleware&);
};
