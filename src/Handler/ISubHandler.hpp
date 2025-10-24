#pragma once

#include "../Middleware/Core/PipelineContext.hpp"

class ISubHandler {
public:
	virtual ~ISubHandler() {}

	virtual HttpResponse handle(PipelineContext &ctx) = 0;
};
