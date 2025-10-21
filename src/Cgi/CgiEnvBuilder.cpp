#include "CgiEnvBuilder.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Server/Client.hpp"
#include <algorithm>
#include <sstream>

namespace {
const std::vector< std::string >
createEnvpArray(const std::map< std::string, std::string > &envMap) {
	std::vector< std::string > envpStrs;
	envpStrs.reserve(envMap.size());

	std::map< std::string, std::string >::const_iterator it = envMap.begin();
	for (; it != envMap.end(); ++it) {
		envpStrs.push_back(it->first + "=" + it->second);
	}
	return envpStrs;
}

bool isValidEnvValue(const std::string &value) {
	const size_t MAX_ENV_VALUE_SIZE = 8192;

	if (value.size() > MAX_ENV_VALUE_SIZE) {
		return false;
	}

	for (size_t i = 0; i < value.size(); ++i) {
		const unsigned char c = static_cast< unsigned char >(value[i]);
		if (c == 0) {
			return false;
		}
	}
	return true;
}
} // namespace

std::vector< std::string > CgiEnvBuilder::build(const PipelineContext &ctx,
												const std::string &scriptPath) {
	const HttpRequest &req = ctx.req;
	std::map< std::string, std::string > envMap;

	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["SERVER_PROTOCOL"] = req.getVersion();
	envMap["REQUEST_METHOD"] = req.getMethod();
	envMap["SCRIPT_FILENAME"] = scriptPath;
	envMap["SCRIPT_NAME"] = req.getPath();
	envMap["SERVER_SOFTWARE"] = ctx.conf.getAppInfo().httpServerName;
	envMap["SERVER_NAME"] = req.getHeader("Host");
	envMap["SERVER_PORT"] =
		StringOps::toString(ctx.ownerClient.getServerPort());
	envMap["REMOTE_ADDR"] = ctx.ownerClient.getIp();
	envMap["REMOTE_PORT"] = StringOps::toString(ctx.ownerClient.getPort());
	envMap["QUERY_STRING"] = req.getQueriesString();

	if (req.hasHeader("Content-Type")) {
		const std::string &contentType = req.getHeader("Content-Type");
		if (isValidEnvValue(contentType)) {
			envMap["CONTENT_TYPE"] = contentType;
		}
	}
	if (req.hasHeader("Content-Length")) {
		const std::string &contentLength = req.getHeader("Content-Length");
		if (isValidEnvValue(contentLength)) {
			envMap["CONTENT_LENGTH"] = contentLength;
		}
	}

	const std::map< std::string, std::vector< std::string > > &headers =
		req.getHeaders();
	for (std::map< std::string, std::vector< std::string > >::const_iterator
			 it = headers.begin();
		 it != headers.end(); ++it) {
		if (it->second.empty()) {
			continue;
		}

		const std::string &headerValue = it->second.front();

		if (!isValidEnvValue(headerValue)) {
			continue;
		}

		std::string httpHeader = "HTTP_" + it->first;
		std::replace(httpHeader.begin(), httpHeader.end(), '-', '_');
		std::transform(httpHeader.begin(), httpHeader.end(), httpHeader.begin(),
					   ::toupper);
		envMap[httpHeader] = headerValue;
	}

	return createEnvpArray(envMap);
}
