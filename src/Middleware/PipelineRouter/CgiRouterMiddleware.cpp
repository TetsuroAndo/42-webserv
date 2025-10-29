#include "CgiRouterMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Resolver/RequestResolver.hpp"
#include "../../Lib/Logger/Log.hpp"

CgiRouterMiddleware::CgiRouterMiddleware() { _cgiHandler = new CgiHandler(); }

CgiRouterMiddleware::~CgiRouterMiddleware() { delete _cgiHandler; }

/**
 * @brief リクエストがCGI実行対象か（拡張子と設定が一致するか）を判定
 */
bool CgiRouterMiddleware::isCgiRequest(PipelineContext &ctx,
									   const Location &loc) const {
	if (loc.cgiConf.empty()) {
		return false;
	}

	std::string scriptVirtual;
	std::string pathInfo;
	if (!RequestResolver::extractCgiScript(ctx.req.getPath(), loc,
										   scriptVirtual, pathInfo)) {
		return false;
	}

	// scriptVirtual の拡張子でCGI対象か判定
	const size_t dotPos = scriptVirtual.rfind('.');
	if (dotPos == std::string::npos) {
		return false;
	}
	const std::string ext = scriptVirtual.substr(dotPos);
	return loc.cgiConf.count(ext) > 0;
}

void CgiRouterMiddleware::handle(PipelineContext &ctx,
								 MiddlewareProcessor *proc) {
	const Location &loc = ctx.conf.getLocation(ctx.req.getPath());

	if (isCgiRequest(ctx, loc)) {
		LOG(DEBUG) << "CgiRouterMiddleware: Detected CGI request."
				   << attr("path", ctx.req.getPath());

		const std::string &method = ctx.req.getMethod();

		// CGIで許可するメソッドか？ (GET/POSTのみ)
		if (method != "GET" && method != "POST") {
			LOG(WARNING) << "CgiRouterMiddleware: Method not allowed for CGI."
						 << attr("method", method);
			ctx.res.setStatusCode(HttpStatus::METHOD_NOT_ALLOWED);
			ctx.res.setHeader("Allow", "GET, POST");
			return;
		}

		if (loc.allowedMethods.count(method) == 0) {
			LOG(WARNING)
				<< "CgiRouterMiddleware: Method not allowed by location config."
				<< attr("method", method);
			ctx.res.setStatusCode(HttpStatus::METHOD_NOT_ALLOWED);
			return;
		}

		// CgiHandlerに処理を委譲（CGIプロセス起動）
		if (_cgiHandler == NULL) {
			LOG(ERROR) << "CgiHandler is NULL";
			ctx.res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
			return;
		}
		try {
			ctx.res = _cgiHandler->handle(ctx);

			if (ctx.res.getStatusCode() < 400) {
				// Server::handleClientReadが即時レスポンスを返さないようフラグを立てる
				ctx.isCgi = true;
			} else {
				// CGI実行失敗時はサブパイプラインを終了し、上位に戻す
				return;
			}
		} catch (const std::exception &e) {
			LOG(ERROR) << "CgiHandler failed with exception: " << e.what();
			ctx.res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
			return;
		}
	} else {
		proc->next(ctx);
	}
}
