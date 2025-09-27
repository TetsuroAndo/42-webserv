#include "PipelineRouteBuilder.hpp"
#include "../Core/PipelineContext.hpp"
#include "../Primary/PipelineRouterMiddleware.hpp"
#include "../Primary/RequestParserMiddleware.hpp"
#include "../Secondary/HandlerMiddleware.hpp"
#include "../Secondary/SessionMiddleware.hpp"
#include "../../Handler/StaticFileHandler.hpp"
#include "../../Handler/DeleteHandler.hpp"
#include "../../Handler/CgiHandler.hpp"

PipelineRouteBuilder::PipelineRouteBuilder() {}

PipelineRouteBuilder::~PipelineRouteBuilder() {
	for (size_t i = 0; i < _createdProcessors.size(); ++i) {
		delete _createdProcessors[i];
	}
}

void PipelineRouteBuilder::buildRoute(const Config &conf, MiddlewareProcessor *mainProc) {
	RouteMap routes;
	const std::map<std::string, Location>& locations = conf.getLocations();

	for (std::map<std::string, Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		const Location &currentLocation = it->second;
		std::map<std::string, ISubHandler*> handlers;

		MiddlewareProcessor *routeProcessor = new MiddlewareProcessor();
		_createdProcessors.push_back(routeProcessor);

		if (!currentLocation.allowedMethods.empty()) {
			routeProcessor->addMiddleware(new SessionMiddleware( /* TODO: Implement SessionMiddleware */ ));
		}

		if (currentLocation.allowedMethods.count("GET")) {
			handlers["GET"] = new StaticFileHandler( /* TODO: Implement location config for GET */ );
		}
		if (currentLocation.allowedMethods.count("HEAD")) {
			handlers["HEAD"] = new StaticFileHandler( /* TODO: Implement HEAD method */ );
		}
		if (currentLocation.allowedMethods.count("POST")) {
			handlers["POST"] = new CgiHandler();
		}
		if (currentLocation.allowedMethods.count("DELETE")) {
			handlers["DELETE"] = new DeleteHandler();
		}
		if (!handlers.empty()) {
			routeProcessor->addMiddleware(new HandlerMiddleware(handlers));
		}
		routes[currentLocation.path] = routeProcessor;
	}

	mainProc->addMiddleware(new RequestParserMiddleware());
	mainProc->addMiddleware(new PipelineRouterMiddleware(routes));
}
