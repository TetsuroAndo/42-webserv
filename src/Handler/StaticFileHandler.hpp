#pragma once

#include "ISubHandler.hpp"

class StaticFileHandler : public ISubHandler {
public:
	StaticFileHandler();
	~StaticFileHandler();

	virtual HttpResponse handle(PipelineContext &ctx);

private:
	void generateDirectoryListing(PipelineContext &ctx,
								  const std::string &directoryPath,
								  const std::string &requestPath);

	StaticFileHandler(const StaticFileHandler &);
	StaticFileHandler &operator=(const StaticFileHandler &);
};
