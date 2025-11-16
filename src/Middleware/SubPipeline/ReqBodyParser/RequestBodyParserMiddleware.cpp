#include "RequestBodyParserMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Http/Parser/ParseResult.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../Core/PipelineContext.hpp"

void RequestBodyParserMiddleware::handle(PipelineContext &ctx,
										 MiddlewareProcessor *proc) {
	RequestParser &parser = ctx.parser;

	// パーサーの状態がボディパーシングの状態でない場合はスキップ
	if (parser.getState() != RequestParser::STATE_BODY) {
		if (proc) {
			proc->next(ctx);
		}
		return;
	}

	// Location固有のボディサイズ制限を設定（これは正しい位置です）
	const Location loc = ctx.conf.getLocation(ctx.req.getPath());
	if (loc.hasMaxRequestBodySize) {
		ctx.req.setMaxBodySize(loc.maxRequestBodySize);
	}

	// パーサーマネージャーにボディ解析を委譲
	ParseResult result = parser.parseBody(ctx.req, ctx.recvBuffer);

	switch (result) {
	case PARSE_INCOMPLETE: {
		// ボディのパーシングが完了していない場合は待機
		return;
	}
	case PARSE_ERROR: {
		ctx.res.setStatusCode(parser.getErrorCode());
		return;
	}
	case PARSE_HEADERS_COMPLETE: {
		// ボディパーシング中にヘッダー完了が返ることはない（エラーとして扱う）
		parser.setErrorCode(HttpStatus::BAD_REQUEST);
		ctx.res.setStatusCode(parser.getErrorCode());
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
