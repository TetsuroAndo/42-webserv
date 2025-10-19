#pragma once

#include "../Config/Config.hpp"
#include "Core/IMiddleware.hpp"

class RedirectMiddleware : public IMiddleware {
public:
	RedirectMiddleware(const std::map< std::string, Redirect > &redirects);
	~RedirectMiddleware();
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *next);

private:
	const std::map< std::string, Redirect > _redirects;

	std::string buildRedirectUrl(const HttpRequest &req,
								 const std::string &toUrl,
								 const std::string &matchedPath,
								 const std::string &requestPath);

	RedirectMiddleware(const RedirectMiddleware &other);
	RedirectMiddleware &operator=(const RedirectMiddleware &other);
};
