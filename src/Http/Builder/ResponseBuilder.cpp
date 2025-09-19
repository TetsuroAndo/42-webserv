#include "../Core/HttpResponse.hpp"
#include "../Core/HttpStatus.hpp"
#include "../../Lib/Time/TimeCache.hpp"
#include "ResponseBuilder.hpp"
#include <sstream>
#include <ctime>

ResponseBuilder::ResponseBuilder() {}

std::string ResponseBuilder::build(HttpResponse& res) {
	addDateHeader(res);
	addServerHeader(res);
	addContentLengthHeader(res);

	std::ostringstream oss;

	// Status Line
	oss << res.getVersion() << " " << res.getStatusCode() << " "
		<< HttpStatus::getReason(res.getStatusCode()) << "\r\n";

	// Headers
	const std::map<std::string, std::string>& headers = res.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}

	oss << "\r\n";
	oss << res.getBody();
	return oss.str();
}

void ResponseBuilder::addDateHeader(HttpResponse& res) {
	if (!res.hasHeader("Date")) {
		res.setHeader("Date", TimeCache::getCurrentTime());
	}
}

void ResponseBuilder::addServerHeader(HttpResponse& res) {
	if (!res.hasHeader("Server")) {
		res.setServerName();
	}
}

void ResponseBuilder::addContentLengthHeader(HttpResponse& res) {
	if (!res.hasHeader("Content-Length")) {
		std::ostringstream oss;
		oss << res.getBody().length();
		res.setHeader("Content-Length", oss.str());
	}
}
