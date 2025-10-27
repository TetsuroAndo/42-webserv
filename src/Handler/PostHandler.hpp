#pragma once

#include "ISubHandler.hpp"

class PostHandler : public ISubHandler {
public:
	PostHandler();
	~PostHandler();

	HttpResponse handle(PipelineContext &ctx);

private:
};
