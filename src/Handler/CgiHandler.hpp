#pragma once

#include "ISubHandler.hpp"

class CgiHandler : public ISubHandler {
public:
	CgiHandler();
	~CgiHandler();

	HttpResponse handle(const HttpRequest& req, const Config& config);

private:


	CgiHandler(const CgiHandler&);
	CgiHandler& operator=(const CgiHandler&);
};
