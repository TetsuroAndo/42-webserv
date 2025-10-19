#pragma once

#include "../../../Cgi/CgiManager.hpp"
#include "../../Core/IMiddleware.hpp"
#include <map>
#include <string>

class ISubHandler;

class HandlerMiddleware : public IMiddleware {
public:
	HandlerMiddleware(CgiManager *cgiManager);
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *next);

private:
	CgiManager *_cgiManager;
	std::string getAllowedMethods();

	HandlerMiddleware(const HandlerMiddleware &);
	HandlerMiddleware &operator=(const HandlerMiddleware &);
};
