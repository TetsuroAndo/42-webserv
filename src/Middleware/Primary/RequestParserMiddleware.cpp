#include "RequestParserMiddleware.hpp"
#include "../../Http/Parser/RequestParser.hpp" // 仮にパーサーがあるとする

void RequestParserMiddleware::handle(PipelineContext &ctx, MiddlewareProcessor *proc) {
    if (!ctx.req || !ctx.res) {
        return;
    }

    // recvBufferからHttpRequestをパース
    RequestParser parser;
    bool parseOk = parser.parse(*(ctx.req), ctx.recvBuffer);

    if (parseOk) {
        // パース成功 → 次のミドルウェアへ
        if (proc) {
            proc->next(ctx);
        }
    } else {
        // パース失敗 → エラーレスポンスをセットして終了
        ctx.res->setStatusCode(400); // Bad Request
        ctx.res->setBody("<html><body><h1>400 Bad Request</h1></body></html>");
        //ctx.sendBuffer = ctx.res->getResponse();
        // この場合、次のミドルウェアには進まない
    }
}
