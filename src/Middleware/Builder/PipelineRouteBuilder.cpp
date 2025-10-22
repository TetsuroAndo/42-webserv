#include "PipelineRouteBuilder.hpp"
#include "../../Handler/DeleteHandler.hpp"
#include "../../Handler/PostHandler.hpp"
#include "../../Handler/StaticFileHandler.hpp"
#include "../PipelineRouter/PipelineRouterMiddleware.hpp"
#include "../PipelineRouter/Session/SessionMiddleware.hpp"
#include "../PipelineRouter/handler/HandlerMiddleware.hpp"
#include "../RedirectMiddleware.hpp"
#include "../RequestParser/RequestParserMiddleware.hpp"
// #include "../../Handler/CgiHandler.hpp"

PipelineRouteBuilder::PipelineRouteBuilder() {}

PipelineRouteBuilder::~PipelineRouteBuilder() {
	for (size_t i = 0; i < _createdProcessors.size(); ++i) {
		delete _createdProcessors[i];
	}
}

void PipelineRouteBuilder::buildRoute(const Config &conf,
									  MiddlewareProcessor *mainProc) {
	RouteMap routes;
	const std::map< std::string, Location > &locations = conf.getLocations();

	for (std::map< std::string, Location >::const_iterator it =
			 locations.begin();
		 it != locations.end(); ++it) {
		const Location &currentLocation = it->second;
		std::map< std::string, ISubHandler * > handlers;

		MiddlewareProcessor *routeProcessor = new MiddlewareProcessor();
		_createdProcessors.push_back(routeProcessor);

		if (!currentLocation.allowedMethods.empty()) {
			routeProcessor->addMiddleware(new SessionMiddleware());
		}

		if (currentLocation.allowedMethods.count("GET")) {
			handlers["GET"] = new StaticFileHandler();
		}
		if (currentLocation.allowedMethods.count("HEAD")) {
			handlers["HEAD"] = new StaticFileHandler();
		}
		if (currentLocation.allowedMethods.count("POST")) {
			handlers["POST"] = new PostHandler();
		}
		if (currentLocation.allowedMethods.count("DELETE")) {
			handlers["DELETE"] = new DeleteHandler();
		}
		routeProcessor->addMiddleware(new HandlerMiddleware(handlers));
		routes[currentLocation.path] = routeProcessor;
	}

	mainProc->addMiddleware(new RequestParserMiddleware());
	mainProc->addMiddleware(new RedirectMiddleware(conf));
	mainProc->addMiddleware(new PipelineRouterMiddleware(routes));
}
