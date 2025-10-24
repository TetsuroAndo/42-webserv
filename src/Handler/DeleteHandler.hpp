#pragma once

#include "ISubHandler.hpp"

class DeleteHandler : public ISubHandler {
public:
	DeleteHandler();
	~DeleteHandler();

	HttpResponse handle(PipelineContext &ctx);

private:
	DeleteHandler(const DeleteHandler &);
	DeleteHandler &operator=(const DeleteHandler &);
};
