#include "VHostSelectMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Server/Client/Client.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

VHostSelectMiddleware::VHostSelectMiddleware() {}

VHostSelectMiddleware::~VHostSelectMiddleware() {}

/**
 * @brief クライアントのリクエストヘッダに基づいて適切な vhost を確定する
 * 		  Hostヘッダ を確認して最終的な正しい仮想ホストを決定
 * @note Currently, it uses the client's active vhost without Host header
 * parsing.
 */
void VHostSelectMiddleware::handle(PipelineContext &ctx,
								   MiddlewareProcessor *proc) {
	if (ctx.parser.getState() == RequestParser::STATE_REQUEST_LINE ||
		ctx.parser.getState() == RequestParser::STATE_HEADERS) {
		return;
	}

	// TODO: ホストヘッダーに基づく選択はまだ実装されていない
	// ここに Host ヘッダーを解析して適切な vhost を選択するロジックを実装する
	std::string vHostDomain = ctx.req.getHeader("Host");

	ctx.setConfig(ctx.ownerClient.getConfig());

	// エラーチェック: Configが正しくセットされているか
	if (ctx.conf == NULL) {
		ctx.setError(HttpStatus::INTERNAL_SERVER_ERROR);
		if (proc) {
			ErrorHandlerMiddleware errorHandler;
			errorHandler.handle(ctx, proc);
		}
		return;
	}

	/// vhost指定のリクエストヘッダサイズの検証とエラーハンドリング （ホスト確定後のため）
	const size_t headerBytes = ctx.parser.getLastHeaderBytes();
	if (ctx.conf != NULL && ctx.conf->getMaxRequestHeaderSize() < headerBytes) {
		ctx.setError(HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE);
		if (proc) {
			ErrorHandlerMiddleware errorHandler;
			errorHandler.handle(ctx, proc);
		}
		return;
	}

	if (proc) {
		proc->next(ctx);
	}
}
