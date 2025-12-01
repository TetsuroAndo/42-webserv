#include "ErrorHandlerMiddleware.hpp"
#include "../../../Handler/ErrorHandler.hpp"

ErrorHandlerMiddleware::ErrorHandlerMiddleware(const Config &config)
	: _config(config) {}

ErrorHandlerMiddleware::~ErrorHandlerMiddleware() {}

void ErrorHandlerMiddleware::handle(PipelineContext &ctx,
									MiddlewareProcessor *proc) {
	(void)proc;
	HttpResponse &res = ctx.res;
	const int statusCode = res.getStatusCode();

	// 4xx/5xxエラーの場合、error_pagesをチェック
	if (statusCode >= 400 && statusCode <= 599) {
		// ErrorHandlerを使用してカスタムエラーページを適用
		// ErrorHandlerは、error_pagesが設定されている場合はカスタムページを返し、
		// 設定されていない場合はデフォルトのエラーページを返す
		ErrorHandler errorHandler;
		ctx.res = errorHandler.handle(ctx);
		// Set "Connection: close" only for protocol violation or security-related errors
		if (statusCode == 400 || statusCode == 413 || statusCode == 431 ||
		    statusCode == 500 || statusCode == 501 || statusCode == 502 ||
		    statusCode == 503 || statusCode == 504) {
			ctx.res.setHeader("Connection", "close");
		}
	}
	// エラーハンドリング後はパイプラインを終了
}
