#pragma once

#include "ISubHandler.hpp"

class ErrorHandler : public ISubHandler {
public:
	ErrorHandler();
	~ErrorHandler();

	HttpResponse handle(const HttpRequest& req, const Config& config);

private:


	ErrorHandler(const ErrorHandler&);
	ErrorHandler& operator=(const ErrorHandler&);
};
