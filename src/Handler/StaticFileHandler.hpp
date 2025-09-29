#pragma once

#include "ISubHandler.hpp"

class StaticFileHandler : public ISubHandler {
public:
	StaticFileHandler();
	~StaticFileHandler();

	HttpResponse handle(const HttpRequest &req, const Config &config);

private:
	void generateDirectoryListing(HttpResponse &res,
								  const HttpRequest &req,
							      const std::string &directoryPath,
								  const std::string &requestPath);

	StaticFileHandler(const StaticFileHandler &);
	StaticFileHandler &operator=(const StaticFileHandler &);
};
