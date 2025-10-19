#pragma once

#include "ISubHandler.hpp"

class ErrorHandler : public ISubHandler {
public:
	ErrorHandler();
	~ErrorHandler();

	virtual HttpResponse handle(PipelineContext &ctx);

private:
	ErrorHandler(const ErrorHandler &);
	ErrorHandler &operator=(const ErrorHandler &);
};
