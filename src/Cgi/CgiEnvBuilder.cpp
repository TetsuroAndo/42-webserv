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
	if (scriptPath[0] != '/') {
		result += "/";
	}
	result += scriptPath;
	return result;
}

std::string queryString(const HttpRequest &req) {
	const std::map< std::string, std::string > map = req.getQueries();
	std::map< std::string, std::string >::const_iterator it = map.begin();
	std::string result;
	for (; it != map.end(); ++it) {
		result += it->first + "=" + it->second;
		std::map< std::string, std::string >::const_iterator nextIt = ++it;
		if (nextIt != map.end()) {
			result += "&";
		}
	}
	return result;
}

std::string fileName(const std::string &scriptPath) {
	std::string trimmed = scriptPath;
	trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
	trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

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
	if (1 < Authorization.size()) {
		remoteUser = Base64::decode(Authorization[1]);
	}

	Location loc = c.getLocation(requestedPath);

	envMap["AUTH_TYPE"] =
		StringOps::split(req.getHeader("Authorization"), " ")[0];
	envMap["CONTENT_LENGTH"] = StringOps::toString(req.getBody().size());
	envMap["CONTENT_TYPE"] = req.getHeader("Content-Type");
	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["PATH_INFO"] = requestedPath; // Locationsのroot+ファイル名
	envMap["PATH_TRANSLATED"] = ::fullURI(
		c.getAppInfo().httpProtocolVersion, c.getListens()[0].interface,
		StringOps::toString(c.getListens()[0].port),
		ctx.req.getPath()); // リクエストのURIを全文 (文字列操作で作る)
	envMap["QUERY_STRING"] = queryString(ctx.req); // リクエストの?以降をここに
	envMap["REMOTE_ADDR"] = ctx.ownerClient.getIp();
	envMap["REMOTE_HOST"] = ""; // 空文字で登録
	envMap["REMOTE_IDENT"] = ctx.session->getId();
	envMap["REMOTE_USER"] = remoteUser;
	envMap["REQUEST_METHOD"] = req.getMethod();
	envMap["SCRIPT_NAME"] =
		fileName(requestedPath); // まっさらなCGIのファイル名
	envMap["SERVER_NAME"] =
		c.getListens()[0]
			.interface; // Locationsで指定されるIPアドレス(0番目で固定)
	envMap["SERVER_PORT"] = StringOps::toString(
		c.getListens()[0]
			.port); // Locationsのうち、scriptPathが属す場所のポート(0番目で固定)
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
