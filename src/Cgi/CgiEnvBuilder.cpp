#include "CgiEnvBuilder.hpp"

#include "../Http/Resolver/RequestResolver.hpp"
#include "../Lib/Base64/Base64.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Server/Client.hpp"
#include <algorithm>
#include <map>

namespace {
std::vector< std::string >
createEnvpArray(const std::map< std::string, std::string > &_env) {
	std::vector< std::string > envpStrs;
	envpStrs.reserve(_env.size());

	std::map< std::string, std::string >::const_iterator it = _env.begin();
	for (; it != _env.end(); ++it) {
		envpStrs.push_back(it->first + "=" + it->second);
	}
	return envpStrs;
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

/**
 * @brief PATH_INFOを抽出する
 * @param fullPath リクエストのフルパス
 * @return PATH_INFO部分の文字列
 */
std::string extractPathInfo(std::string fullPath) {
	try {
		const std::size_t dotPos = fullPath.find('.');
		const std::size_t slashPos = fullPath.substr(dotPos).find('/');
		std::string trim =
			fullPath.substr(dotPos, std::string::npos).substr(slashPos);
		return trim;
	} catch (...) {
		return "";
	}
}

/// @brief HTTPヘッダーキーをCGI環境変数名形式 (大文字 + アンダースコア)
/// に変換する
std::string formatHeaderKeyForCgi(std::string key) {
	StringOps::toUpper(key);
	std::replace(key.begin(), key.end(), '-', '_');
	return key;
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

	std::string hostName = req.hasHeader("Host") ? req.getHeader("Host") : "";
	int port = ctx.ownerClient.getListenPort();
	const Location loc = c.getLocation(hostName, port, req.getPath());

	const std::vector< ServerConfig > &servers = c.getServers();
	if (servers.empty()) {
		LOG(ERROR) << "CgiEnvBuilder: No server configuration found";
		return std::vector< std::string >();
	}

	// 適切なserverを選択
	const ServerConfig &server = c.getServerConfig(hostName, port);
	if (server.listens.empty()) {
		LOG(ERROR) << "CgiEnvBuilder: No listen configuration found";
		return std::vector< std::string >();
	}

	const Listen &listen = server.listens[0];

	std::map< std::string, std::string > env;
	env["AUTH_TYPE"] = authType;
	env["CONTENT_LENGTH"] = StringOps::toString(req.getBody().size());
	env["CONTENT_TYPE"] = req.getHeader("Content-Type");
	env["GATEWAY_INTERFACE"] = ctx.conf.getAppInfo().cgiVersion;
	// PATH_INFO を取得
	const std::string pathInfo = extractPathInfo(req.getPath());
	env["PATH_INFO"] = pathInfo;
	// PATH_TRANSLATED は PATH_INFO をファイルシステムパスに解決したもの
	env["PATH_TRANSLATED"] =
		pathInfo.empty() ? "" : RequestResolver::resolvePath(pathInfo, c);
	env["QUERY_STRING"] = queryString(ctx.req); // リクエストの?以降をここに
	env["REMOTE_ADDR"] = ctx.ownerClient.getIp();
	env["REMOTE_HOST"] = "";
	env["REMOTE_IDENT"] = ctx.session ? ctx.session->getId() : "";
	env["REMOTE_USER"] = remoteUser;
	env["REQUEST_METHOD"] = req.getMethod();
	env["SCRIPT_NAME"] = fileName(requestedPath);
	env["SERVER_NAME"] = listen.interface;
	env["SERVER_PORT"] = StringOps::toString(listen.port);
	env["SERVER_PROTOCOL"] = c.getAppInfo().httpProtocolVersion;
	env["SERVER_SOFTWARE"] = ctx.conf.getAppInfo().softwareName;
	env["REMOTE_PORT"] = StringOps::toString(ctx.ownerClient.getPort());

	// HTTPヘッダーを環境変数に変換
	_headerToEnvMap(req, env);

	if (ctx.session != NULL) {
		env["HTTP_X_WEBSERV_SESSION_ID"] = ctx.session->getId();
	}

	// 毎回は見なくていいデバッグだけど、まだ消さないで〜
	// std::cout << "Env map created: \n";
	// for (std::map< std::string, std::string >::const_iterator it =
	// env.begin(); 	 it != env.end(); ++it) { 	std::cout << "  " <<
	// it->first <<
	// "=" << it->second << "\n";
	// }
	// std::cout << std::endl;
	return createEnvpArray(env);
}

void CgiEnvBuilder::_headerToEnvMap(const HttpRequest &req,
									std::map< std::string, std::string > &env) {
	const std::map< std::string, std::vector< std::string > > &headers =
		req.getHeaders();

	for (std::map< std::string, std::vector< std::string > >::const_iterator
			 it = headers.begin();
		 it != headers.end(); ++it) {

		const std::string &key = it->first;
		const std::vector< std::string > &values = it->second;

		if (StringOps::equalsIgnoreCase(key, "Content-Length") ||
			StringOps::equalsIgnoreCase(key, "Content-Type") ||
			StringOps::equalsIgnoreCase(key, "Authorization")) {
			continue;
		}

		// ヘッダーが存在する場合、CGI形式で環境変数に追加
		if (!values.empty()) {
			// キーをCGI形式 (HTTP_COOKIE など) に変換
			std::string cgiKey = "HTTP_" + formatHeaderKeyForCgi(key);

			// HttpRequestの実装に従い、複数ヘッダーがある場合は最後の値を使用
			env[cgiKey] = values.back();
		}
	}
}
