#pragma once

#include "ISubHandler.hpp"

class StaticFileHandler : public ISubHandler {
public:
	StaticFileHandler();
	~StaticFileHandler();

	HttpResponse handle(const HttpRequest& req, const Config& config);

private:


	StaticFileHandler(const StaticFileHandler&);
	StaticFileHandler& operator=(const StaticFileHandler&);
};
