#include "SessionMiddleware.hpp"

#include "../../Core/MiddlewareProcessor.hpp"
#include "../../Core/PipelineContext.hpp"

SessionMiddleware::SessionMiddleware() {}

SessionMiddleware::~SessionMiddleware() {}

// TODO: これの実装進める
// TODO: ctxがsessionを持ってるので、そこをctxのreqから読み取る。レスポンスにセッション情報をつける。
void SessionMiddleware::handle(PipelineContext &ctx,
							   MiddlewareProcessor *proc) {
	(void)ctx;
	if (proc) {
		proc->next(ctx);
	}
}
