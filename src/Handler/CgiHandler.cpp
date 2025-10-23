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
	HttpResponse &response = *ctx.res;

	if (response.getStatusCode() >= 400) {
		return response;
	}
	response.setIsCgi(true);
	return response;
}
