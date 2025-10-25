#include "CgiEnvBuilder.hpp"

#include "../Handler/HandlerUtil.hpp"
#include "../Lib/Base64/Base64.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Server/Client.hpp"
#include <map>

namespace {
std::vector< std::string >
createEnvpArray(const std::map< std::string, std::string > &envMap) {
	std::vector< std::string > envpStrs;
	envpStrs.reserve(envMap.size());

	std::map< std::string, std::string >::const_iterator it = envMap.begin();
	for (; it != envMap.end(); ++it) {
		envpStrs.push_back(it->first + "=" + it->second);
	}
	return envpStrs;
}

std::string fullURI(const std::string &version, const std::string &ip,
					const std::string &port, const std::string &scriptPath) {
	std::string result;
	const std::string modifiedVersion =
		StringOps::toLower(StringOps::trim(version, "0123456789. /"));

	result += modifiedVersion + "://";
	result += ip + ":" + port;
	if (!scriptPath.empty() && scriptPath[0] != '/') {
		result += "/";
	}
	result += scriptPath;
	return result;
}

std::string queryString(const HttpRequest &req) {
	const std::map< std::string, std::string > map = req.getQueries();
	std::string result;
	for (std::map< std::string, std::string >::const_iterator it = map.begin();
		 it != map.end();) {
		result += it->first + "=" + it->second;
		++it;
		if (it != map.end()) {
			result += "&";
		}
	}
	return result;
}

std::string fileName(const std::string &scriptPath) {
	std::string trimmed = scriptPath;
	const size_t start = trimmed.find_first_not_of(" \t\n\r");
	if (start != std::string::npos) {
		trimmed.erase(0, start);
	} else {
		trimmed.clear();
		return "/";
	}
	const size_t end = trimmed.find_last_not_of(" \t\n\r");
	if (end != std::string::npos) {
		trimmed.erase(end + 1);
	}

	std::size_t pos = trimmed.find('?');
	if (pos != std::string::npos)
		trimmed = trimmed.substr(0, pos);
	pos = trimmed.find_last_of('/');
	const std::string name =
		(pos != std::string::npos) ? trimmed.substr(pos + 1) : trimmed;

	return "/" + name;
}
} // namespace

/**
 * @brief PipelineContextからCGI環境変数のリストを生成する
 * RFC 3875
 * 参考: https://4judgement.github.io/rfc-translater/html/rfc3875.html
 *
 * @param ctx リクエストのコンテキスト
 * @param requestedPath リクエストのパス
 * @return "KEY=VALUE"形式のvector
 */
std::vector< std::string >
CgiEnvBuilder::build(const PipelineContext &ctx,
					 const std::string &requestedPath) {
	const Config &c = ctx.conf;
	const HttpRequest &req = ctx.req;
	std::map< std::string, std::string > envMap;

	const std::vector< std::string > Authorization =
		StringOps::split(req.getHeader("Authorization"), " ");
	std::string remoteUser = "";
	std::string authType = "";
	if (!Authorization.empty()) {
		authType = Authorization[0];
	}
	if (1 < Authorization.size()) {
		remoteUser = Base64::decode(Authorization[1]);
	}

	Location loc = c.getLocation(requestedPath);

	if (c.getListens().empty()) {
		LOG(ERROR) << "CgiEnvBuilder: No listen configuration found";
		return std::vector< std::string >();
	}

	const Listen &listen = c.getListens()[0];

	envMap["AUTH_TYPE"] = authType;
	envMap["CONTENT_LENGTH"] = StringOps::toString(req.getBody().size());
	envMap["CONTENT_TYPE"] = req.getHeader("Content-Type");
	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["PATH_INFO"] = requestedPath;
	envMap["PATH_TRANSLATED"] =
		::fullURI(c.getAppInfo().httpProtocolVersion, listen.interface,
				  StringOps::toString(listen.port), ctx.req.getPath());
	envMap["QUERY_STRING"] = queryString(ctx.req);
	envMap["REMOTE_ADDR"] = ctx.ownerClient.getIp();
	envMap["REMOTE_HOST"] = "";
	envMap["REMOTE_IDENT"] = ctx.session ? ctx.session->getId() : "";
	envMap["REMOTE_USER"] = remoteUser;
	envMap["REQUEST_METHOD"] = req.getMethod();
	envMap["SCRIPT_NAME"] = fileName(requestedPath);
	envMap["SERVER_NAME"] = listen.interface;
	envMap["SERVER_PORT"] = StringOps::toString(listen.port);
	envMap["SERVER_PROTOCOL"] = c.getAppInfo().httpProtocolVersion;
	envMap["SERVER_SOFTWARE"] = ctx.conf.getAppInfo().softwareName;
	envMap["REMOTE_PORT"] = StringOps::toString(ctx.ownerClient.getPort());

	// 毎回は見なくていいデバッグだけど、まだ消さないで〜
	// LOG(DEBUG) << "Env map created: ";
	// for (std::map< std::string, std::string >::const_iterator it =
	// 		 envMap.begin();
	// 	 it != envMap.end(); ++it) {
	// 	LOG(DEBUG) << "  " << it->first << "=" << it->second;
	// }

	return createEnvpArray(envMap);
}
