#pragma once

#include "../Config/Config.hpp"
#include "../Http/Core/HttpRequest.hpp"

class IMiddleware {
public:
	virtual ~IMiddleware() {}
	virtual void handle(HttpRequest& req, const Config& conf, class IMiddleware* next) = 0;
};
