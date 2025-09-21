#include "MiddlewareProcessor.hpp"
#include <iostream>

MiddlewareProcessor::MiddlewareProcessor() : _currentIndex(0) {}

MiddlewareProcessor::~MiddlewareProcessor() {
	for (size_t i = 0; i < _middlewares.size(); ++i) {
		delete _middlewares[i];
	}
	_middlewares.clear();
}

void MiddlewareProcessor::addMiddleware(IMiddleware *middleware) {
	_middlewares.push_back(middleware);
}

// 処理パイプラインを開始
void MiddlewareProcessor::handle(PipelineContext &ctx) {
	_currentIndex = 0;
	next(ctx);
}

/**
 * @brief 次のミドルウェアを呼び出す
 *
*/
void MiddlewareProcessor::next(PipelineContext &ctx) {
	if (_currentIndex < _middlewares.size()) {
		IMiddleware *currentMiddleware = _middlewares[_currentIndex];
		++_currentIndex;
		currentMiddleware->handle(ctx, this);
	}
}
