#pragma once

#include "../Core/HttpResponse.hpp"
#include <string>

class ResponseBuilder {
public:
	static std::string build(HttpResponse& res);

private:
	static void addDateHeader(HttpResponse& res);
	static void addServerHeader(HttpResponse& res);
	static void addContentLengthHeader(HttpResponse& res);
	static void addMimeTypeHeader(HttpResponse& res);

	ResponseBuilder();
	ResponseBuilder(const ResponseBuilder&);
	ResponseBuilder& operator=(const ResponseBuilder&);
	~ResponseBuilder();
};
