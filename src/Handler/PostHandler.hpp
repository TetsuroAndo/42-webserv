#pragma once

#include "ISubHandler.hpp"

class PostHandler : public ISubHandler {
public:
	PostHandler();
	~PostHandler();

	virtual HttpResponse handle(PipelineContext &ctx);

private:
};
