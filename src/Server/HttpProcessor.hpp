#pragma once

#include <string>
#include "../Core/HttpRequest.hpp"
#include "../Config/Config.hpp"

class HttpProcessor {
public:
	HttpProcessor();
	~HttpProcessor();

	void handle(HttpRequest &ctx, const Config &conf);
	void next(HttpRequest &ctx, const Config &conf);

private:

};
