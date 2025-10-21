#include "ResponseBuilder.hpp"
#include "../../Lib/Time/TimeCache.hpp"
#include "../Core/HttpStatus.hpp"
#include <sstream>

std::string ResponseBuilder::build(HttpResponse &res) {
	std::ostringstream oss;
	oss << res.getVersion() << " " << res.getStatusCode() << " "
		<< HttpStatus::getReason(res.getStatusCode()) << "\r\n";

	addDateHeader(res);
	addServerHeader(res);
	addContentLengthHeader(res);

	const std::map< std::string, std::vector< std::string > > &headers =
		res.getHeaders();
	for (std::map< std::string, std::vector< std::string > >::const_iterator
			 it = headers.begin();
		 it != headers.end(); ++it) {
		for (std::vector< std::string >::const_iterator valIt =
				 it->second.begin();
			 valIt != it->second.end(); ++valIt) {
			oss << it->first << ": " << *valIt << "\r\n";
		}
	}
	oss << "\r\n";
	oss << res.getBody();
	return oss.str();
}

void ResponseBuilder::addDateHeader(HttpResponse &res) {
	if (!res.hasHeader("Date")) {
		res.setHeader("Date", TimeCache::getHeaderTimestamp());
	}
}

void ResponseBuilder::addServerHeader(HttpResponse &res) {
	if (!res.hasHeader("Server")) {
		res.setHeader("Server", "webserv/1.0"); // ToDo: Get from config
	}
}

void ResponseBuilder::addContentLengthHeader(HttpResponse &res) {
	if (!res.hasHeader("Content-Length")) {
		std::ostringstream oss;
		oss << res.getBody().length();
		res.setHeader("Content-Length", oss.str());
	}
}
