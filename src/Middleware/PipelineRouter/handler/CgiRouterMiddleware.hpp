#pragma once

#include "../../Core/IMiddleware.hpp"

class CgiManager;

class CgiRouterMiddleware : public IMiddleware {
public:
	CgiRouterMiddleware(CgiManager *cgiManager);
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *next);

private:
	CgiManager *_cgiManager;

	CgiRouterMiddleware(const CgiRouterMiddleware &);
	CgiRouterMiddleware &operator=(const CgiRouterMiddleware &);
};
