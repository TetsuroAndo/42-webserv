#pragma once

#include "../../Handler/CgiHandler.hpp"
#include "../Core/IMiddleware.hpp"

class CgiRouterMiddleware : public IMiddleware {
private:
	CgiHandler *_cgiHandler;

	/**
	 * @brief リクエストがCGI実行対象か（拡張子と設定が一致するか）を判定
	 */
	bool isCgiRequest(const PipelineContext &ctx, const Location &loc) const;

public:
	CgiRouterMiddleware();
	virtual ~CgiRouterMiddleware();

	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

private:
	CgiRouterMiddleware(const CgiRouterMiddleware &);
	CgiRouterMiddleware &operator=(const CgiRouterMiddleware &);
};
