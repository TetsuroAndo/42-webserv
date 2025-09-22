#include "PipelineRouteBuilder.hpp"
#include "../Primary/RequestParserMiddleware.hpp"
#include "../Secondary/HandlerMiddleware.hpp"


PipelineRouteBuilder::PipelineRouteBuilder() {}

PipelineRouteBuilder::~PipelineRouteBuilder() {}

void PipelineRouteBuilder::buildRoute(const Config &conf, MiddlewareProcessor *proc) {
    (void)conf;
    proc->addMiddleware(new RequestParserMiddleware());
    proc->addMiddleware(new HandlerMiddleware());
}
