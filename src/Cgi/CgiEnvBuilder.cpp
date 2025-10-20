#include "CgiEnvBuilder.hpp"

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
	(void)ctx;
	(void)scriptPath;
	std::map< std::string, std::string > envMap;

	// TODO: Implement CGI environment variable generation
	// envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	// envMap["SERVER_PROTOCOL"] = req.getVersion();
	// envMap["REQUEST_METHOD"] = req.getMethod();
	// envMap["SCRIPT_FILENAME"] = scriptPath;
	// envMap["SCRIPT_NAME"] = req.getPath();
	// envMap["SERVER_SOFTWARE"] = ctx.conf.getAppInfo().httpServerName;
	// envMap["SERVER_NAME"] = req.getHeader("Host");
	// envMap["SERVER_PORT"] = "80";
	// envMap["REMOTE_ADDR"] = ctx.ownerClient.getIp();
	// envMap["REMOTE_PORT"] = StringOps::toString(ctx.ownerClient.getPort());
	// envMap["QUERY_STRING"] = req.getQueriesString();
	// envMap["CONTENT_TYPE"] = req.getHeader("Content-Type");
	// envMap["CONTENT_LENGTH"] = req.getHeader("Content-Length");

	return createEnvpArray(envMap);
}
