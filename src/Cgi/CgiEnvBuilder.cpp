#include "CgiEnvBuilder.hpp"

#include "../Handler/HandlerUtil.hpp"
#include "../Lib/Base64/Base64.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Server/Client.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
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
	if (1 < Authorization.size()) {
		remoteUser = Base64::decode(Authorization[1]);
	}

	const Location loc = c.getLocation(req.getPath());

	std::map< std::string, std::string > env;
	env["AUTH_TYPE"] = StringOps::split(req.getHeader("Authorization"), " ")[0];
	env["CONTENT_LENGTH"] = StringOps::toString(req.getBody().size());
	env["CONTENT_TYPE"] = req.getHeader("Content-Type");
	env["GATEWAY_INTERFACE"] = ctx.conf.getAppInfo().cgiVersion;
	env["PATH_INFO"] =
		extractPathInfo(ctx.req.getPath()); // cgiのパス以降のパス
	env["PATH_TRANSLATED"] =
		env["PATH_INFO"].empty()
			? ""
			: HandlerUtil::resolvePath(env["PATH_INFO"],
									   c);		// PATH_INFOを取得するURI
	env["QUERY_STRING"] = queryString(ctx.req); // リクエストの?以降をここに
	env["REMOTE_ADDR"] = ctx.ownerClient.getIp();
	env["REMOTE_HOST"] = ""; // 空文字で登録
	if (ctx.session)
		env["REMOTE_IDENT"] = ctx.session->getId();
	env["REMOTE_USER"] = remoteUser;
	env["REQUEST_METHOD"] = req.getMethod();
	env["SCRIPT_NAME"] = fileName(requestedPath); // まっさらなCGIのファイル名
	env["SERVER_NAME"] =
		c.getListens()[0]
			.interface; // Locationsで指定されるIPアドレス(0番目で固定)
	env["SERVER_PORT"] = StringOps::toString(
		c.getListens()[0]
			.port); // Locationsのうち、scriptPathが属す場所のポート(0番目で固定)
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
	// env.begin(); 	 it != env.end(); ++it) { 	std::cout << "  " <<
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
