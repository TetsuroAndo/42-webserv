#include "RedirectMiddleware.hpp"

RedirectMiddleware::RedirectMiddleware(const Config& config)
    : _config(config) {}

RedirectMiddleware::~RedirectMiddleware() {}

void RedirectMiddleware::handle(PipelineContext& ctx, MiddlewareProcessor* proc) {
    const HttpRequest& req = *ctx.req;
    const Redirect& redirect = _config.getRedirect(req.getPath());

    if (!redirect.fromPath.empty()) {
        // Redirect found
        HttpResponse res(ctx.conf);
        res.setStatusCode(redirect.code);

        std::string newLocation = redirect.toUrl;
        // Append remaining path if it's a prefix match
        if (req.getPath().length() > redirect.fromPath.length()) {
            newLocation += req.getPath().substr(redirect.fromPath.length());
        }
        // Append query string
        const std::map<std::string, std::string>& queries = req.getQueries();
        if (!queries.empty()) {
            newLocation += "?";
            for (std::map<std::string, std::string>::const_iterator it = queries.begin(); it != queries.end(); ++it) {
                newLocation += it->first + "=" + it->second;
                std::map<std::string, std::string>::const_iterator next_it = it;
                ++next_it;
                if (next_it != queries.end()) {
                    newLocation += "&";
                }
            }
        }
        res.setHeader("Location", newLocation);
        *ctx.res = res;
        // Stop further processing
        return;
    }

    // No redirect, continue to next middleware
    proc->next(ctx);
}
