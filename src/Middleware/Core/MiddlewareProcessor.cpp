#include "IMiddleware.hpp"

#include "MiddlewareProcessor.hpp"

MiddlewareProcessor::MiddlewareProcessor() : _currentIndex(0) {}

MiddlewareProcessor::~MiddlewareProcessor() {
	for (size_t i = 0; i < _middlewareChain.size(); ++i) {
		delete _middlewareChain[i];
	}
	_middlewareChain.clear();
}

void MiddlewareProcessor::addMiddleware(IMiddleware *middleware) {
	_middlewareChain.push_back(middleware);
}

// 処理パイプラインを開始
void MiddlewareProcessor::handle(PipelineContext &ctx) {
	_currentIndex = 0;
	next(ctx);
}

/**
 * @brief 次のミドルウェアを呼び出す
*/
void MiddlewareProcessor::next(PipelineContext &ctx) {
	if (_currentIndex < _middlewareChain.size()) {
		IMiddleware *currentMiddleware = _middlewareChain[_currentIndex];
		++_currentIndex;
		currentMiddleware->handle(ctx, this);
	}
}
