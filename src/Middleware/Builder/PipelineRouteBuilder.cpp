#include "PipelineRouteBuilder.hpp"
#include "../Core/PipelineContext.hpp"
#include "../Primary/PipelineRouterMiddleware.hpp"
#include "../Primary/RequestParserMiddleware.hpp"
#include "../Secondary/HandlerMiddleware.hpp"
#include "../../Handler/StaticFileHandler.hpp"
#include "../../Handler/DeleteHandler.hpp"
// #include "../../Handler/CgiHandler.hpp" // 将来のCGI用

PipelineRouteBuilder::PipelineRouteBuilder() {}

PipelineRouteBuilder::~PipelineRouteBuilder() {
	for (size_t i = 0; i < _createdProcessors.size(); ++i) {
		delete _createdProcessors[i];
	}
}

void PipelineRouteBuilder::buildRoute(const Config &conf, MiddlewareProcessor *mainProc) {
	MiddlewareProcessor* staticProc = createStaticRouteProcessor(conf);
	MiddlewareProcessor* deleteProc = createDeleteRouteProcessor(conf);
	// MiddlewareProcessor* cgiProc = createCgiRouteProcessor(conf);

	RouteMap routes;
	routes["/uploads"] = deleteProc; // /uploads は専用のdeleteProcが処理
	routes["/"] = staticProc;        // /uploads に一致しなかったものは全てstaticProcが処理
	// routes["/cgi-bin"] = cgiProc;

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
