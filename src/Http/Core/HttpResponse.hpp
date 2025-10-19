#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../../Config/Config.hpp"
#include <map>
#include <string>
#include <vector>

class HttpResponse {
public:
	std::string version;
	bool isCgi;
	int statusCode;
	std::string statusMessage;
	std::string body;
	std::map< std::string, std::string > headers;

	HttpResponse();

	// Append a header value (useful for Set-Cookie and other multi-value
	// headers)
	void appendHeader(const std::string &key, const std::string &value);
};

#endif
