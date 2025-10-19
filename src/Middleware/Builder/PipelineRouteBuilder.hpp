#pragma once

#include "../../Config/Config.hpp"
#include "../Core/MiddlewareProcessor.hpp"
#include <map>
#include <string>

typedef std::map< std::string, class MiddlewareProcessor * > RouteMap;

#include "../../Cgi/CgiManager.hpp"

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
	void buildRoute(const Config &conf, CgiManager *cgiManager,
					MiddlewareProcessor *mainProc);

private:
	std::vector< MiddlewareProcessor * > _createdProcessors;

	PipelineRouteBuilder(const PipelineRouteBuilder &);
	PipelineRouteBuilder &operator=(const PipelineRouteBuilder &);
};
