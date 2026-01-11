#include "VHostSelectMiddleware.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Server/Client/Client.hpp"
#include "../../Server/Server.hpp"
#include "../../Server/VHost/VHostResolver.hpp"
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

	const std::string hostHeader = ctx.req.getHeader("Host");
	VirtualHost *selected = VHostResolver::find(
		ctx.ownerClient.getServer().getVhosts(),
		ctx.ownerClient.getListenKey(), hostHeader);
	if (selected != NULL) {
		const Config &activeConfig = ctx.ownerClient.getConfig();
		if (&activeConfig != &selected->config) {
			ctx.ownerClient.setActiveVhost(*selected);
			ctx.setConfig(selected->config);
			if (proc) {
				ctx.ownerClient.getMainProcessor().handle(ctx);
			}
			return;
		}
		ctx.setConfig(selected->config);
	} else {
		ctx.setConfig(ctx.ownerClient.getConfig());
	}

	// エラーチェック: Configが正しくセットされているか
	if (ctx.conf == NULL) {
		ctx.setError(HttpStatus::INTERNAL_SERVER_ERROR);
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
