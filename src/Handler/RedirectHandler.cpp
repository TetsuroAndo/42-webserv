#include "RedirectHandler.hpp"

RedirectHandler::RedirectHandler(const std::string &redirectUrl, int statusCode)
	: _redirectUrl(redirectUrl), _statusCode(statusCode) {}

RedirectHandler::~RedirectHandler() {}

HttpResponse RedirectHandler::handle(PipelineContext &ctx) {
	HttpResponse &res = *ctx.res;

	res.setStatusCode(_statusCode);
	res.setHeader("Location", _redirectUrl);
	return res;
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
