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

	HttpResponse res;
	res.isCgi = true;
	return res;
}
