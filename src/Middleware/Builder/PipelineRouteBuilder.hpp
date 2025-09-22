#pragma once

#include "../../Config/Config.hpp"
#include "../Core/MiddlewareProcessor.hpp"
#include <string>
#include <map>

typedef std::map<std::string, class MiddlewareProcessor*> RouteMap;

class PipelineRouteBuilder {
public:
	PipelineRouteBuilder();
	~PipelineRouteBuilder();

	void buildRoute(const Config &conf, MiddlewareProcessor *proc);

private:
	PipelineRouteBuilder(const PipelineRouteBuilder&);
	PipelineRouteBuilder& operator=(const PipelineRouteBuilder&);
};
