#include "VHostSelectMiddleware.hpp"
#include "../../Server/Client/Client.hpp"

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

	ctx.setConfig(ctx.ownerClient.getConfig());

	if (proc) {
		proc->next(ctx);
	}
}
