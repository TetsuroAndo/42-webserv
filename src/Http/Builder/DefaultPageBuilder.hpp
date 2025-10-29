#pragma once

#include "../Core/HttpResponse.hpp"
#include <string>

class DefaultPageBuilder {
public:
	static void generateSimpleBody(const std::string &method, HttpResponse &res,
							   const int code,
							   const std::string &description = "",
							   const bool skipIfAlreadySet = false);
};
