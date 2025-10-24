#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include <string>
#include <vector>

class CgiEnvBuilder {
public:
	CgiEnvBuilder();
	~CgiEnvBuilder();

	/**
	 * @brief PipelineContextからCGI環境変数のリストを生成する
	 * @param ctx リクエストのコンテキスト
	 * @param requestedPath 実行するCGIスクリプトのフルパス
	 * @return "KEY=VALUE"形式の文字列配列
	 */
	std::vector< std::string > build(const PipelineContext &ctx,
									 const std::string &requestedPath);

private:
	std::map< std::string, std::string > _env;

	void _headerToEnvMap(const HttpRequest &req);

	CgiEnvBuilder(const CgiEnvBuilder &);
	CgiEnvBuilder &operator=(const CgiEnvBuilder &);
};
