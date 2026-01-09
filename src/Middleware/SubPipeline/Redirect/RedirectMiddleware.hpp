#pragma once

#include "../../../Handler/RedirectHandler.hpp"
#include "../../../Http/Core/HttpRequest.hpp"
#include "../../../Http/Core/HttpResponse.hpp"
#include "../../Core/IMiddleware.hpp"

class RedirectMiddleware : public IMiddleware {
public:
	RedirectMiddleware();
	~RedirectMiddleware();

	void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

	RedirectMiddleware(const RedirectMiddleware &other);
	RedirectMiddleware &operator=(const RedirectMiddleware &other);
};
