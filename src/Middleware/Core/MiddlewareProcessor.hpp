#pragma once

#include "../../Config/Config.hpp"
#include "../../Http/Core/HttpRequest.hpp"
#include "PipelineContext.hpp"
#include <string>
#include <vector>

class IMiddleware;

class MiddlewareProcessor {
public:
	MiddlewareProcessor();
	~MiddlewareProcessor();

	/**
	 * @brief _middlewareChainに*middlewareを追加する
	 * @param middleware 追加するMiddleware
	 */
	void addMiddleware(IMiddleware *middleware);

	/**
	 * @brief Middlewareの処理を開始する
	 * _currentIndexを0にリセットし、next()を呼び出す
	 * @note PipelineContext内のHttpRequest, HttpResponse, Config, Sessionは
	 * 事前にセットされている必要がある
	 * @param ctx パイプラインコンテキスト
	 */
	void handle(PipelineContext &ctx);

	/**
	 * @brief 次のMiddlewareの処理をChainの終端まで実行する
	 * _middlewareChainの_currentIndex番目のMiddlewareのhandle()を呼び出す
	 * @note Middlewareのhandle()内で必要に応じてnext()を呼び出すこと
	 * @param ctx パイプラインコンテキスト
	 */
	void next(PipelineContext &ctx);

private:
	std::vector< IMiddleware * > _middlewareChain;
	size_t _currentIndex;
};
