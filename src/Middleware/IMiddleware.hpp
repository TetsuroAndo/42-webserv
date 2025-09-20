#pragma once

#include "../Config/Config.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Server/HttpProcessor.hpp"

class IMiddleware {
public:
	virtual ~IMiddleware() {}
	virtual void handle(HttpRequest& req, const Config& conf, HttpProcessor& processor) = 0;
};
