#pragma once

#include "../../Core/IMiddleware.hpp"

class ISubHandler;

class HandlerMiddleware : public IMiddleware
{
public:
    HandlerMiddleware(const std::map<std::string, ISubHandler*>& handlers);
    ~HandlerMiddleware();
    virtual void handle(PipelineContext& ctx, MiddlewareProcessor* proc);

private:
    std::map<std::string, ISubHandler*> _handlers;

    std::string getAllowedMethods();
    HandlerMiddleware(const HandlerMiddleware&);
    HandlerMiddleware& operator=(const HandlerMiddleware&);
};
