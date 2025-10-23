#include "PipelineRouteBuilder.hpp"
#include "../../Handler/DeleteHandler.hpp"
#include "../../Handler/PostHandler.hpp"
#include "../../Handler/StaticFileHandler.hpp"
#include "../PipelineRouter/CgiRouterMiddleware.hpp"
#include "../PipelineRouter/PipelineRouterMiddleware.hpp"
#include "../RequestParser/RequestParserMiddleware.hpp"
#include "../SubPipeline/Handler/HandlerMiddleware.hpp"
#include "../SubPipeline/Redirect/RedirectMiddleware.hpp"
#include "../SubPipeline/Session/SessionMiddleware.hpp"
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
		MiddlewareProcessor *routeProcessor = new MiddlewareProcessor();
		_createdProcessors.push_back(routeProcessor);

		if (!currentLocation.allowedMethods.empty()) {
			routeProcessor->addMiddleware(new SessionMiddleware());
		}

		// CgiRouterMiddleware
		if (!currentLocation.cgiConf.empty()) {
			routeProcessor->addMiddleware(new CgiRouterMiddleware());
		}

		// HandlerMiddleware (静的ファイル・アップロード・削除用)
		// CgiRouterMiddlewareを通過したリクエスト(＝CGIではない)のみが処理される
		std::map< std::string, ISubHandler * > staticHandlers;
		if (currentLocation.allowedMethods.count("GET")) {
			staticHandlers["GET"] = new StaticFileHandler();
		}
		if (currentLocation.allowedMethods.count("HEAD")) {
			staticHandlers["HEAD"] = new StaticFileHandler();
		}
		if (currentLocation.allowedMethods.count("POST")) {
			staticHandlers["POST"] = new PostHandler();
		}
		if (currentLocation.allowedMethods.count("DELETE")) {
			staticHandlers["DELETE"] = new DeleteHandler();
		}

		if (!staticHandlers.empty()) {
			routeProcessor->addMiddleware(
				new HandlerMiddleware(staticHandlers));
		}

		routes[currentLocation.path] = routeProcessor;
	}

	mainProc->addMiddleware(new RequestParserMiddleware());
	mainProc->addMiddleware(new RedirectMiddleware(conf));
	mainProc->addMiddleware(new PipelineRouterMiddleware(routes));
}
