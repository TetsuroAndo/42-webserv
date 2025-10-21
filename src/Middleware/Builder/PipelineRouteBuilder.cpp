#include "PipelineRouteBuilder.hpp"
#include "../PipelineRouter/handler/AllowedMethodsMiddleware.hpp"
#include "../PipelineRouter/handler/CgiRouterMiddleware.hpp"
#include "../PipelineRouter/handler/HttpHandlerMiddleware.hpp"
#include "../RedirectMiddleware.hpp"
#include "../RequestParser/RequestParserMiddleware.hpp"

PipelineRouteBuilder::PipelineRouteBuilder() {}
PipelineRouteBuilder::~PipelineRouteBuilder() {}

void PipelineRouteBuilder::buildRoute(const Config &config,
									  CgiManager *cgiManager,
									  MiddlewareProcessor *processor) {
	processor->addMiddleware(new RequestParserMiddleware());
	processor->addMiddleware(new RedirectMiddleware(config.getRedirects()));
	processor->addMiddleware(new AllowedMethodsMiddleware());
	processor->addMiddleware(new CgiRouterMiddleware(cgiManager));
	processor->addMiddleware(new HttpHandlerMiddleware());
}
