#include "CgiEnvBuilder.hpp"

#include "../Lib/StringOps/StringOps.hpp"
#include "../Server/Client.hpp"

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
} // namespace

/**
 * @brief PipelineContextからCGI環境変数のリストを生成する
 * RFC 3875
 * 参考: https://4judgement.github.io/rfc-translater/html/rfc3875.html
 *
 * @param ctx リクエストのコンテキスト
 * @param scriptPath 実行するCGIスクリプトのフルパス
 * @return "KEY=VALUE"形式の文字列ベクトル
 */
std::vector< std::string > CgiEnvBuilder::build(const PipelineContext &ctx,
												const std::string &scriptPath) {
	const Config &c = ctx.conf;
	const HttpRequest &req = *ctx.req;
	std::map< std::string, std::string > envMap;

	// TODO: 足りない要素をパーサーで解析して埋める

	std::vector< std::string > Authorization =
		StringOps::split(req.getHeader("Authorization"), " ");
	std::string remoteUser = "";
	if (1 <= Authorization.size()) {
		remoteUser = Authorization[1]; // TODO:BASE64でデコードする
	}

	envMap["AUTH_TYPE"] =
		StringOps::split(req.getHeader("Authorization"), " ")[0];
	envMap["CONTENT_LENGTH"] = req.getBody().size();
	envMap["CONTENT_TYPE"] = req.getHeader("Content-Type");
	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["PATH_INFO"] = "";		// Locationsのroot+ファイル名
	envMap["PATH_TRANSLATED"] = ""; // リクエストのURIを全文 (文字列操作で作る)
	envMap["QUERY_STRING"] = "";	// リクエストの?以降をここに
	envMap["REMOTE_ADDR"] = ctx.ownerClient.getIp();
	envMap["REMOTE_HOST"] =
		""; // 空文字で登録(digコマンドを実行する必要あるため)
	envMap["REMOTE_IDENT"] = ctx.session->getId();
	envMap["REMOTE_USER"] = remoteUser;
	envMap["REQUEST_METHOD"] = req.getMethod();
	envMap["SCRIPT_NAME"] = ""; // まっさらなCGIのファイル名
	envMap["SERVER_NAME"] = ""; // Locationsで指定されるIPアドレス
	envMap["SERVER_PORT"] = ""; // Locationsのうち、scriptPathが属す場所のポート
	envMap["SERVER_PROTOCOL"] = c.getAppInfo().httpProtocolVersion;
	envMap["SERVER_SOFTWARE"] = ctx.conf.getAppInfo().softwareName;
	envMap["REMOTE_PORT"] = StringOps::toString(ctx.ownerClient.getPort());
	return createEnvpArray(envMap);
}
