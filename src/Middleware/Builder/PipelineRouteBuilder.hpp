#pragma once

#include "../../Config/Config.hpp"
#include "../Core/MiddlewareProcessor.hpp"
#include <string>
#include <map>

typedef std::map<std::string, class MiddlewareProcessor*> RouteMap;

class PipelineRouteBuilder {
public:
	PipelineRouteBuilder();
	~PipelineRouteBuilder();

	/**
	 * @brief Middleware Chainを構築する
	 * Configに基づいてMiddlewareを構築し、ルーティングを設定する
	 * @param conf サーバ設定
	 * @param proc ミドルウェアプロセッサの先頭ノード
	 */
	void buildRoute(const Config &conf, MiddlewareProcessor *mainProc);

private:
	/** 
	 * @brief 静的ファイル処理用のパイプラインを構築する
	 */
	MiddlewareProcessor* createStaticRouteProcessor(const Config& conf);
	/** 
	 * @brief DELETEリクエスト処理用のパイプラインを構築する
	 */
	MiddlewareProcessor* createDeleteRouteProcessor(const Config& conf);
	// /** 
	//  * @brief CGIリクエスト処理用のパイプラインを構築する
	//  */
	// MiddlewareProcessor* createCgiRouteProcessor(const Config& conf); // 将来のCGI用

	// メモリ管理用のポインタ
	std::vector<MiddlewareProcessor*> _createdProcessors;

	PipelineRouteBuilder(const PipelineRouteBuilder&);
	PipelineRouteBuilder& operator=(const PipelineRouteBuilder&);
};
