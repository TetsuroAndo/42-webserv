#pragma once

#include "../Core/IMiddleware.hpp"

class ISubHandler;

class HandlerMiddleware : public IMiddleware {
  public:
	HandlerMiddleware();
	~HandlerMiddleware();
	virtual void handle(PipelineContext &ctx, MiddlewareProcessor *proc);

  private:
	ISubHandler *_staticFileHandler;
	ISubHandler *_deleteHandler;
	// ISubHandler *_cgiHandler;
};
