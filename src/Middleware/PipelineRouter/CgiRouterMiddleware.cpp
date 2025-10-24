#include "CgiRouterMiddleware.hpp"
#include "../../Handler/HandlerUtil.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Lib/Logger/Log.hpp"

CgiRouterMiddleware::CgiRouterMiddleware() { _cgiHandler = new CgiHandler(); }

CgiRouterMiddleware::~CgiRouterMiddleware() { delete _cgiHandler; }

/**
 * @brief リクエストがCGI実行対象か（拡張子と設定が一致するか）を判定
 */
bool CgiRouterMiddleware::isCgiRequest(PipelineContext &ctx,
									   const Location &loc) const {
	const std::string &path = ctx.req.getPath();

	if (loc.cgiConf.empty()) {
		return false;
	}

	const size_t dotPos = path.rfind('.');
	if (dotPos == std::string::npos) {
		return false;
	}

	const std::string ext = path.substr(dotPos);
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
			HandlerUtil::generateSimpleBody(method, ctx.res,
											HttpStatus::METHOD_NOT_ALLOWED,
											"Method Not Allowed for CGI");
			ctx.res.setHeader("Allow", "GET, POST");
			return;
		}

		if (loc.allowedMethods.count(method) == 0) {
			LOG(WARNING)
				<< "CgiRouterMiddleware: Method not allowed by location config."
				<< attr("method", method);
			HandlerUtil::generateSimpleBody(method, ctx.res,
											HttpStatus::METHOD_NOT_ALLOWED);
			return;
		}

		// CgiHandlerに処理を委譲（CGIプロセス起動）
		try {
			ctx.res = _cgiHandler->handle(ctx);

			if (ctx.res.getStatusCode() < 400) {
				// Server::handleClientReadが即時レスポンスを返さないようフラグを立てる
				ctx.isCgi = true;
			}
			// 起動失敗時は、ctx.resに設定されたエラーがそのままレスポンスされる
		} catch (const std::exception &e) {
			LOG(ERROR) << "CgiHandler failed with exception: " << e.what();
			HandlerUtil::generateSimpleBody(method, ctx.res,
											HttpStatus::INTERNAL_SERVER_ERROR);
		}
		return;

	} else {
		proc->next(ctx);
	}
}
