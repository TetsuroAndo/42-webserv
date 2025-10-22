#pragma once

#include "../Config/Config.hpp"
#include "../Handler/RedirectHandler.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "Core/IMiddleware.hpp"

class RedirectMiddleware : public IMiddleware {
public:
	RedirectMiddleware(const Config &config);
	~RedirectMiddleware();

	void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
	const Config &_config;

	RedirectMiddleware(const RedirectMiddleware &other);
	RedirectMiddleware &operator=(const RedirectMiddleware &other);
};
