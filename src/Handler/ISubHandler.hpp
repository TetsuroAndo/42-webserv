#pragma once

#include "../Config/Config.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"

class ISubHandler {
public:
	virtual ~ISubHandler() {}

	virtual HttpResponse handle(const HttpRequest &req, HttpResponse &res,
								const Config &config) = 0;
};
