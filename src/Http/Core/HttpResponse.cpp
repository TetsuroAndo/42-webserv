#include "HttpResponse.hpp"

HttpResponse::HttpResponse()
	: version("HTTP/1.1"), isCgi(false), statusCode(0) {}

void HttpResponse::appendHeader(const std::string &key,
								const std::string &value) {
	std::map< std::string, std::string >::iterator it = headers.find(key);
	if (it != headers.end()) {
		// Append to existing header value with proper HTTP format
		it->second += "\r\n" + key + ": " + value;
	} else {
		// Create new header
		headers[key] = value;
	}
}
