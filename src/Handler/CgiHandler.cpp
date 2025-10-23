#include "CgiHandler.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Logger/Log.hpp"
#include "HandlerUtil.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

HttpResponse CgiHandler::handle(PipelineContext &ctx) {
	LOG(INFO) << "CgiHandler processing request";

	ctx.cgiManager.createWorker(ctx);

	if (ctx.res.getStatusCode() >= 400) {
		LOG(INFO) << "CGI setup failed with status code"
				  << attr("status", ctx.res.getStatusCode());
		return ctx.res;
	}

	LOG(INFO) << "CgiHandler: CGI process started successfully";
	return ctx.res;
}
