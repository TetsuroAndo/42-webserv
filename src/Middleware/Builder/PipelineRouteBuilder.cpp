#include "PipelineRouteBuilder.hpp"
#include "../Core/PipelineContext.hpp"
#include "../Primary/PipelineRouterMiddleware.hpp"
#include "../Primary/RequestParserMiddleware.hpp"
#include "../Secondary/HandlerMiddleware.hpp"
#include "../../Handler/StaticFileHandler.hpp"
#include "../../Handler/DeleteHandler.hpp"
// #include "../../Handler/CgiHandler.hpp" //

PipelineRouteBuilder::PipelineRouteBuilder() {}

PipelineRouteBuilder::~PipelineRouteBuilder() {
	for (size_t i = 0; i < _createdProcessors.size(); ++i) {
		delete _createdProcessors[i];
	}
}

void PipelineRouteBuilder::buildRoute(const Config &conf, MiddlewareProcessor *mainProc) {
	RouteMap routes;
	const std::vector<Location>& locations = conf.getLocations();

	for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		const Location &currentLoc = *it;

		MiddlewareProcessor *routeProcessor = new MiddlewareProcessor();
		_createdProcessors.push_back(routeProcessor);

		std::map<std::string, ISubHandler*> handlers;

		if (currentLoc.allowedMethods.count("GET")) {
			handlers["GET"] = new StaticFileHandler( /* TODO: Implement location config for GET */ );
		}
		if (currentLoc.allowedMethods.count("HEAD")) {
			handlers["HEAD"] = new StaticFileHandler( /* TODO: Implement HEAD method */ );
		}
		// if (currentLoc.allowedMethods.count("POST")) {
		// 	handlers["POST"] = new CgiHandler(currentLoc.cgiConf);
		// }
		if (currentLoc.allowedMethods.count("DELETE")) {
			handlers["DELETE"] = new DeleteHandler();
		}
		if (!handlers.empty()) {
			routeProcessor->addMiddleware(new HandlerMiddleware(handlers));
		}
		routes[currentLoc.path] = routeProcessor;
	}

	mainProc->addMiddleware(new RequestParserMiddleware());
	mainProc->addMiddleware(new PipelineRouterMiddleware(routes));
}

MiddlewareProcessor* PipelineRouteBuilder::createStaticRouteProcessor(const Config& conf) {
	(void)conf;
	MiddlewareProcessor* proc = new MiddlewareProcessor();
	_createdProcessors.push_back(proc); // メモリ管理のためベクターに追加

	std::map<std::string, ISubHandler*> handlers;
	handlers["GET"] = new StaticFileHandler();
	handlers["HEAD"] = new StaticFileHandler();
	// handlers["POST"] = new UploadHandler(); 

	proc->addMiddleware(new HandlerMiddleware(handlers));
	return proc;
}

MiddlewareProcessor* PipelineRouteBuilder::createDeleteRouteProcessor(const Config& conf) {
	(void)conf;
	MiddlewareProcessor* proc = new MiddlewareProcessor();
	_createdProcessors.push_back(proc); // メモリ管理のためベクターに追加

	std::map<std::string, ISubHandler*> handlers;
	handlers["DELETE"] = new DeleteHandler();

	proc->addMiddleware(new HandlerMiddleware(handlers));
	return proc;
}
