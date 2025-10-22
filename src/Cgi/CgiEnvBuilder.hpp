#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include <string>
#include <vector>

class CgiEnvBuilder {
public:
	/**
	 * @brief PipelineContextからCGI環境変数のリストを生成する
	 * @param ctx リクエストのコンテキスト
	 * @param requestedPath 実行するCGIスクリプトのフルパス
	 * @return "KEY=VALUE"形式の文字列配列
	 */
	static std::vector< std::string > build(const PipelineContext &ctx,
											const std::string &requestedPath);

private:
	CgiEnvBuilder();
};
