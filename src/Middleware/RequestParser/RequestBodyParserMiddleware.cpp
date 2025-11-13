#include "RequestBodyParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Lib/Logger/Log.hpp"

void RequestBodyParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	// パーサーの状態がボディパーシングの状態でない場合はスキップ
	if (ctx.parser.getState() != RequestParser::STATE_BODY) {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}

	// Location固有のボディサイズ制限を設定
	const Location loc = ctx.conf.getLocation(ctx.req.getPath());
	if (loc.maxRequestBodySize != -1) {
		const std::size_t effectiveLimit =
			static_cast< std::size_t >(loc.maxRequestBodySize);
		ctx.req.setMaxBodySize(effectiveLimit);
	}

	// ボディのパーシングを行う
	// RequestParser内で既にサイズチェックが行われている
	ParseResult result = ctx.parser.parse(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE: {
		// ボディのパーシングが完了していない場合は待機
		return;
	}
	case PARSE_ERROR: {
		ctx.res.setStatusCode(ctx.parser.getErrorCode());
		return;
	}
	case PARSE_HEADERS_COMPLETE: {
		// ボディパーシング中にヘッダー完了が返ることはない（エラーとして扱う）
		ctx.res.setStatusCode(HttpStatus::BAD_REQUEST);
		return;
	}
	case PARSE_COMPLETE: {
		// ボディパーシング完了。次のミドルウェアに進む
		if (proc) {
			proc->next(ctx);
		}
		return;
	}
	}
}
