#include "RedirectMiddleware.hpp"
#include "../Handler/RedirectHandler.hpp"
#include "../Lib/Logger/Log.hpp"

RedirectMiddleware::RedirectMiddleware(
	const std::map< std::string, Redirect > &redirects)
	: _redirects(redirects) {}
RedirectMiddleware::~RedirectMiddleware() {}

void RedirectMiddleware::handle(PipelineContext &ctx,
								MiddlewareProcessor *proc) {
	const HttpRequest &req = ctx.req;
	const std::string &requestPath = req.getPath();

	std::map< std::string, Redirect >::const_iterator it =
		_redirects.find(requestPath);
	if (it != _redirects.end()) {
		std::string redirectUrl =
			buildRedirectUrl(req, it->second.toUrl, requestPath, requestPath);
		RedirectHandler handler(redirectUrl, it->second.code);
		ctx.res = handler.handle(ctx);
		return;
	}

	std::string longestMatch;
	std::map< std::string, Redirect >::const_iterator longestIt =
		_redirects.end();

	for (std::map< std::string, Redirect >::const_iterator iter =
			 _redirects.begin();
		 iter != _redirects.end(); ++iter) {
		const std::string &from = iter->first;
		if (requestPath.find(from) == 0 &&
			from.length() > longestMatch.length()) {
			if (requestPath.length() == from.length() ||
				requestPath[from.length()] == '/') {
				longestMatch = from;
				longestIt = iter;
			}
		}
	}

	if (longestIt != _redirects.end()) {
		std::string redirectUrl = buildRedirectUrl(req, longestIt->second.toUrl,
												   longestMatch, requestPath);

		RedirectHandler handler(redirectUrl, longestIt->second.code);
		ctx.res = handler.handle(ctx);
		return;
	}

	if (proc) {
		proc->next(ctx);
	}
}

std::string RedirectMiddleware::buildRedirectUrl(
	const HttpRequest &req, const std::string &toUrl,
	const std::string &matchedPath, const std::string &requestPath) {
	std::string result = toUrl;

	if (toUrl.find("http://") != 0 && toUrl.find("https://") != 0) {
		if (requestPath.length() > matchedPath.length()) {
			result += requestPath.substr(matchedPath.length());
		}
	}

	std::string queryString = req.getQueriesString();
	if (!queryString.empty()) {
		result += "?" + queryString;
	}

	return result;
}
