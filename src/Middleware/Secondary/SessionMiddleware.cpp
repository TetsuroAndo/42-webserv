#include "../Core/MiddlewareProcessor.hpp"
#include "../Core/PipelineContext.hpp"
#include "SessionMiddleware.hpp"

SessionMiddleware::SessionMiddleware() {}
SessionMiddleware::~SessionMiddleware() {}

void SessionMiddleware::handle(PipelineContext &ctx, MiddlewareProcessor *proc) {
	(void)ctx;
	if (proc) {
		proc->next(ctx);
	}
}
