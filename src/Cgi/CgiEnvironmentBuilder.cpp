#include "CgiEnvironmentBuilder.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "../Server/Client.hpp"
#include "../../Lib/Info/App.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "CgiEnvironmentBuilder.hpp"
// ...

std::vector<std::string> CgiEnvironmentBuilder::build(const PipelineContext &ctx,
													  const Location& locConf,
													  const std::string& scriptPath) {
	(void)locConf;
	std::map<std::string, std::string> envMap;
	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["SERVER_PROTOCOL"] = HTTP_VERSION;
	envMap["REQUEST_METHOD"] = ctx.req.getMethod();
	envMap["SCRIPT_FILENAME"] = scriptPath;
	envMap["SCRIPT_NAME"] = ctx.req.getRequest().getPath();

	// RFC 3875 variables
	envMap["SERVER_SOFTWARE"] = SOFTWARE_NAME;
	envMap["SERVER_NAME"] = req.getHeader("Host"); // Host header
	std::stringstream ss_port;
	ss_port << req.getServerPort();
	envMap["SERVER_PORT"] = ss_port.str();
	envMap["REMOTE_ADDR"] = req.getRemoteAddr();
	std::stringstream ss_remote_port;
	ss_remote_port << req.getRemotePort();
	envMap["REMOTE_PORT"] = ss_remote_port.str();

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
