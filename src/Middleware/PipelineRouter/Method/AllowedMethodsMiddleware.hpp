#pragma once

#include "../../Core/IMiddleware.hpp"
#include <set>
#include <string>

class AllowedMethodsMiddleware : public IMiddleware {
public:
	AllowedMethodsMiddleware();
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *next);

private:
	std::string
	getAllowedMethods(const std::set< std::string > &allowedMethods);

	AllowedMethodsMiddleware(const AllowedMethodsMiddleware &);
	AllowedMethodsMiddleware &operator=(const AllowedMethodsMiddleware &);
};
