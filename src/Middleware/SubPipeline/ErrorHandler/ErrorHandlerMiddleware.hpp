#pragma once

#include "../../../Http/Core/HttpRequest.hpp"
#include "../../../Http/Core/HttpResponse.hpp"
#include "../../Core/IMiddleware.hpp"

class ErrorHandlerMiddleware : public IMiddleware {
public:
	ErrorHandlerMiddleware();
	~ErrorHandlerMiddleware();

	void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

	ErrorHandlerMiddleware(const ErrorHandlerMiddleware &other);
	ErrorHandlerMiddleware &operator=(const ErrorHandlerMiddleware &other);
};
