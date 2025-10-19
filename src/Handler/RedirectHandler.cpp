#include "RedirectHandler.hpp"

RedirectHandler::RedirectHandler(const std::string &redirectUrl, int statusCode)
	: _redirectUrl(redirectUrl), _statusCode(statusCode) {}

RedirectHandler::~RedirectHandler() {}

HttpResponse RedirectHandler::handle(PipelineContext &ctx) {
	(void)ctx.req;
	(void)ctx.conf;
	ctx.res.statusCode = _statusCode;
	ctx.res.headers["Location"] = _redirectUrl;
	return ctx.res;
}

// Private copy constructor and assignment operator to prevent copying
RedirectHandler::RedirectHandler(const RedirectHandler &other)
	: _redirectUrl(other._redirectUrl), _statusCode(other._statusCode) {}

RedirectHandler &RedirectHandler::operator=(const RedirectHandler &other) {
	if (this != &other) {
		_redirectUrl = other._redirectUrl;
		_statusCode = other._statusCode;
	}
	return *this;
}
