#include "../Core/HttpStatus.hpp"
#include "../URI/URI.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "RequestLineParser.hpp"
#include "ParseResult.hpp"
#include <vector>
#include <sstream>

RequestLineParser::RequestLineParser() {}
RequestLineParser::~RequestLineParser() {}

ParseResult RequestLineParser::parse(HttpRequest& request, const std::string& line, int &errorCode) {
	std::vector<std::string> tokens;
	std::string current;
	std::istringstream iss(line);

	while (iss >> current) {
		tokens.push_back(current);
	}
	if (tokens.size() != 3) {
		errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}

	request.setMethod(tokens[0]);
	request.setPath(parsePath(request, tokens[1]));
	request.setVersion(tokens[2]);

	if (request.getVersion() != "HTTP/1.1" && request.getVersion() != "HTTP/1.0") {
		errorCode = HttpStatus::VERSION_NOT_SUPPORTED;
		return PARSE_ERROR;
	}
	return PARSE_COMPLETE;
}

void RequestLineParser::parseQuery(HttpRequest& request, const std::string& queryString) {
	std::istringstream ss(queryString);
	std::string pair;
	while (std::getline(ss, pair, '&')) {
		size_t eq_pos = pair.find('=');
		std::string key, value;
		if (eq_pos != std::string::npos) {
			key = URI::decodeURIComponent(pair.substr(0, eq_pos));
			value = URI::decodeURIComponent(pair.substr(eq_pos + 1));
		} else {
			key = URI::decodeURIComponent(pair);
			value = "";
		}
		request.addQuery(key, value);
	}
}

std::string RequestLineParser::parsePath(HttpRequest& request, const std::string& uri) {
	size_t query_pos = uri.find('?');
	if (query_pos != std::string::npos) {
		std::string path = uri.substr(0, query_pos);
		std::string queryString = uri.substr(query_pos + 1);
		RequestLineParser::parseQuery(request, queryString);
		return path;
	} else {
		return uri;
	}
}
