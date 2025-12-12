#pragma once

#include "ISubHandler.hpp"

class PutHandler : public ISubHandler {
public:
	PutHandler();
	~PutHandler();

	HttpResponse handle(PipelineContext &ctx);

private:
};
