#include "PipelineRouterMiddleware.hpp"
#include "../../Handler/HandlerUtil.hpp"
#include "../../Http/Core/HttpStatus.hpp"

#include <algorithm>
#include <vector>

PipelineRouterMiddleware::PipelineRouterMiddleware(const RouteMap &routes)
	: _routes(routes) {}

PipelineRouterMiddleware::~PipelineRouterMiddleware() {}

struct CompareRoutes {
	bool
	operator()(const std::pair< std::string, MiddlewareProcessor * > &a,
			   const std::pair< std::string, MiddlewareProcessor * > &b) const {
		return a.first.length() > b.first.length();
	}
};

void PipelineRouterMiddleware::handle(PipelineContext &ctx,
									  MiddlewareProcessor *proc) {
	const std::string requestPath = ctx.req.getPath();
	MiddlewareProcessor *nextProcessor = 0;

	std::vector< std::pair< std::string, MiddlewareProcessor * > > sortedRoutes;

	for (RouteMap::const_iterator it = _routes.begin(); it != _routes.end();
		 ++it) {
		sortedRoutes.push_back(*it);
	}

	std::sort(sortedRoutes.begin(), sortedRoutes.end(), CompareRoutes());

	for (std::vector< std::pair< std::string, MiddlewareProcessor * > >::
			 const_iterator it = sortedRoutes.begin();
		 it != sortedRoutes.end(); ++it) {
		const std::string &routeKey = it->first;
		if (requestPath.rfind(routeKey, 0) == 0) {
			nextProcessor = it->second;
			break;
		}
	}

	if (nextProcessor != 0) {
		nextProcessor->handle(ctx);
		if (ctx.res.getStatusCode() >= 400) {
			proc->next(ctx);
		}
	} else {
		ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
		proc->next(ctx);
	}
}
