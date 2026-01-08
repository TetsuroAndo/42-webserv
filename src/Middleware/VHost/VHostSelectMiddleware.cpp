#include "VHostSelectMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Server/Client/Client.hpp"
#include "../SubPipeline/ErrorHandler/ErrorHandlerMiddleware.hpp"

VHostSelectMiddleware::VHostSelectMiddleware() {}

VHostSelectMiddleware::~VHostSelectMiddleware() {}

/**
 * @brief Selects the appropriate virtual host based on the client's request.
 * 		  uses the Host header to determine the correct vhost.
 * @note Currently, it uses the client's active vhost without Host header parsing.
 */
void VHostSelectMiddleware::handle(PipelineContext &ctx,
								   MiddlewareProcessor *proc) {
	// TODO: Host header based selection is not implemented yet.
	if (ctx.parser.getState() == RequestParser::STATE_REQUEST_LINE ||
		ctx.parser.getState() == RequestParser::STATE_HEADERS) {
		return;
	}

	// ここで host ヘッダーを解析して適切な vhost を選択するロジックを実装する

	ctx.setConfig(ctx.ownerClient.getConfig());

	/// @brief vhost指定のリクエストヘッダサイズの検証 （ホスト確定後のため）
	const size_t headerBytes = ctx.parser.getLastHeaderBytes();
	if (ctx.conf != NULL &&
		ctx.conf->getMaxRequestHeaderSize() < headerBytes) {
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
