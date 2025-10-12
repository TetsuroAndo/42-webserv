#include "CgiEnvironmentBuilder.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "../../Lib/StringOps/StringOps.hpp"
#include "CgiEnvironmentBuilder.hpp"
// ...

std::vector<std::string> CgiEnvironmentBuilder::build(const HttpRequest& req,
													  const Location& locConf,
													  const std::string& scriptPath) {
	(void)locConf;
	std::map<std::string, std::string> envMap;
	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
	envMap["REQUEST_METHOD"] = req.getMethod();
	envMap["SCRIPT_FILENAME"] = scriptPath;
	envMap["SCRIPT_NAME"] = req.getPath();

	std::string queryString;
	const std::map<std::string, std::string> &queries = req.getQueries();
	for (std::map<std::string, std::string>::const_iterator it = queries.begin(); it != queries.end(); ++it) {
		if (it != queries.begin()) {
			queryString += "&";
		}
		queryString += it->first + "=" + it->second;
	}
	envMap["QUERY_STRING"] = queryString;

	envMap["CONTENT_TYPE"] = req.getHeader("Content-Type");
	std::stringstream ss;
	ss << req.getBody().length();
	envMap["CONTENT_LENGTH"] = ss.str();
	const std::map<std::string, std::string> &headers = req.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::string envKey = "HTTP_" + it->first;
		std::replace(envKey.begin(), envKey.end(), '-', '_');
		StringOps::toUpper(envKey);
		envMap[envKey] = it->second;
	}

	std::vector<std::string> envpStrs;
	envpStrs.reserve(envMap.size());
	for (std::map<std::string, std::string>::iterator it = envMap.begin(); it != envMap.end(); ++it) {
		envpStrs.push_back(it->first + "=" + it->second);
	}
	return envpStrs;
}
