#include "CgiHandler.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Logger/Log.hpp"
#include "HandlerUtil.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

CgiHandler::CgiHandler(CgiManager *cgiManager) : _cgiManager(cgiManager) {}

CgiHandler::~CgiHandler() {}

HttpResponse CgiHandler::handle(PipelineContext &ctx) {
	LOG(INFO) << "CgiHandler processing request";

	FdEventChanges changes = _cgiManager->createWorker(ctx);

	ctx.addChanges(changes);

	int statusCode = ctx.res.getStatusCode();
	if (statusCode >= 400) {
		return ctx.res;
	}

	ctx.res.setIsCgi(true);
	return ctx.res;
}
