#pragma once

#include "../../../Config/Config.hpp"
#include "../../../Http/Core/HttpRequest.hpp"
#include "../../../Http/Core/HttpResponse.hpp"
#include "../../Core/IMiddleware.hpp"

class ErrorHandlerMiddleware : public IMiddleware {
public:
	ErrorHandlerMiddleware(const Config &config);
	~ErrorHandlerMiddleware();

	void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
	const Config &_config;

	ErrorHandlerMiddleware(const ErrorHandlerMiddleware &other);
	ErrorHandlerMiddleware &operator=(const ErrorHandlerMiddleware &other);
};
