#include "RedirectMiddleware.hpp"
#include <iostream>

RedirectMiddleware::RedirectMiddleware(const Config &config)
	: _config(config) {}

RedirectMiddleware::~RedirectMiddleware() {}

void RedirectMiddleware::handle(PipelineContext &ctx,
								MiddlewareProcessor *proc) {
	const HttpRequest &req = ctx.req;
	const Redirect &redirect = _config.getRedirect(req.getPath());

	if (!redirect.fromPath.empty()) {
		// Redirect found
		ctx.res.setStatusCode(redirect.code);
		ctx.res.setHeader("Content-Type", "text/html");
		ctx.res.setHeader("Content-Length", "0");

		std::string newLocation = redirect.toUrl;
		// Append remaining path if it's a prefix match
		if (req.getPath().length() > redirect.fromPath.length()) {
			newLocation += req.getPath().substr(redirect.fromPath.length());
		}
		// Append query string
		const std::map< std::string, std::string > &queries = req.getQueries();
		if (!queries.empty()) {
			newLocation += "?";
			for (std::map< std::string, std::string >::const_iterator it =
					 queries.begin();
				 it != queries.end(); ++it) {
				newLocation += it->first + "=" + it->second;
				std::map< std::string, std::string >::const_iterator next_it =
					it;
				++next_it;
				if (next_it != queries.end()) {
					newLocation += "&";
				}
			}
		}
		// Convert relative path to absolute URL if needed
		if (newLocation.length() > 0 && newLocation[0] == '/') {
			std::string host = req.getHeader("host");
			if (!host.empty()) {
				newLocation = "http://" + host + newLocation;
			}
		}
		ctx.res.setHeader("Location", newLocation);
		ctx.res.setBody("");
		// Stop further processing
		return;
	}

	// No redirect, continue to next middleware
	proc->next(ctx);
}
