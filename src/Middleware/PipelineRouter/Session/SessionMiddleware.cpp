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
	// TODO: ヘッダーがセッション情報を持ってるか見る
	std::string reqCookieRaw = ctx.req->getHeader("");
	// TODO: ヘッダーのCookie情報をパースする
	// TODO: セッションが有効なものかを確認する
	// TODO: セッションがなければ新しく作る
	// TODO: セッション情報を保存する
	// TODO: セッション情報をレスポンスにつける。


	if (proc) {
		proc->next(ctx);
	}
}
