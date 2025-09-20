#pragma once

#include <string>
#include "../Core/HttpRequest.hpp"
#include "../Config/Config.hpp"

class HttpProcessor {
public:
	HttpProcessor();
	~HttpProcessor();

	void processRequest(const HttpRequest &request);
private:

};
