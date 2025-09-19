#pragma once

#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Config/Config.hpp"

class IHandler {
public:
	virtual ~IHandler() {}
	virtual HttpResponse handle(const HttpRequest& req, const Config& config) = 0;
};
