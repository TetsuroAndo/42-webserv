#include "ResponseBuilder.hpp"
#include "../../Config/PerformanceConfig.hpp"
#include "../../Lib/Time/TimeCache.hpp"
#include "../Core/HttpStatus.hpp"
#include <sstream>

std::string ResponseBuilder::build(HttpResponse &res) {
	std::ostringstream oss;

	// Pre-calculate approximate size to reduce allocations
	size_t estimatedSize = RESPONSE_RESERVE_SIZE + res.body.size();
	for (std::map< std::string, std::string >::const_iterator it =
			 res.headers.begin();
		 it != res.headers.end(); ++it) {
		estimatedSize += it->first.size() + it->second.size() + 4; // ": \r\n"
	}

	oss << res.version << " " << res.statusCode << " "
		<< HttpStatus::getReason(res.statusCode) << "\r\n";

	addDateHeader(res);
	addServerHeader(res);
	addContentLengthHeader(res);

	for (std::map< std::string, std::string >::const_iterator it =
			 res.headers.begin();
		 it != res.headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	oss << res.body;
	return oss.str();
}

void ResponseBuilder::addDateHeader(HttpResponse &res) {
	if (res.headers.find("Date") == res.headers.end()) {
		res.headers["Date"] = TimeCache::getHeaderTimestamp();
	}
}

void ResponseBuilder::addServerHeader(HttpResponse &res) {
	if (res.headers.find("Server") == res.headers.end()) {
		res.headers["Server"] = "webserv/1.0"; // ToDo: Get from config
	}
}

void ResponseBuilder::addContentLengthHeader(HttpResponse &res) {
	if (res.headers.find("Content-Length") == res.headers.end()) {
		std::ostringstream oss;
		oss << res.body.length();
		res.headers["Content-Length"] = oss.str();
	}
}
