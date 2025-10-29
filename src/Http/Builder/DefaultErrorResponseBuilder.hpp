#pragma once

#include "../Core/HttpResponse.hpp"
#include <string>

class DefaultErrorResponseBuilder {
public:
	static void generateSimpleBody(const std::string &method, HttpResponse &res,
							const int code, const std::string &description);
};
