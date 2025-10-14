#pragma once

#include "ISubHandler.hpp"

class DeleteHandler : public ISubHandler {
public:
	DeleteHandler();
	~DeleteHandler();

	HttpResponse handle(const HttpRequest &req, HttpResponse &res,
						const Config &config);

private:
	DeleteHandler(const DeleteHandler &);
	DeleteHandler &operator=(const DeleteHandler &);
};
