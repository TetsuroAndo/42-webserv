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
	const Config &c = ctx.conf;
	const HttpRequest &req = *ctx.req;
	const HttpResponse &res = *ctx.res;

	(void)c;
	(void)req;
	(void)res;
	std::map< std::string, std::string > envMap;

	(void)scriptPath;

	envMap["GATEWAY_INTERFACE"] = "";
	envMap["SERVER_PROTOCOL"] = "";
	envMap["REQUEST_METHOD"] = "";
	envMap["SCRIPT_FILENAME"] = "";
	envMap["SCRIPT_NAME"] = "";
	envMap["SERVER_SOFTWARE"] = "";
	envMap["SERVER_NAME"] = "";
	envMap["SERVER_PORT"] = "";
	envMap["REMOTE_ADDR"] = "";
	envMap["REMOTE_PORT"] = "";
	envMap["QUERY_STRING"] = "";
	envMap["CONTENT_TYPE"] = "";
	envMap["CONTENT_LENGTH"] = "";

	return createEnvpArray(envMap);
}
