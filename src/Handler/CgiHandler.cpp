#include "CgiHandler.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "HandlerUtil.hpp"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

HttpResponse CgiHandler::handle(const HttpRequest &req, const Config &config) {
	HttpResponse res(SERVER_NAME);

	if (req.getMethod() != "POST") {
		HandlerUtil::generateErrorBody(req.getMethod(), res, HttpStatus::METHOD_NOT_ALLOWED);
		return res;
	}

	const Location &loc = config.getLocation(req.getPath());
	std::string uploadStore = loc.uploadStore;

	if (uploadStore.empty()) {
		HandlerUtil::generateErrorBody(req.getMethod(), res, HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	struct stat s;
	if (stat(uploadStore.c_str(), &s) != 0 || !S_ISDIR(s.st_mode)) {
		HandlerUtil::generateErrorBody(req.getMethod(), res, HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	std::string filename = "uploaded_file.bin";
	std::string fullUploadPath = uploadStore + "/" + filename;

	std::ofstream ofs(fullUploadPath.c_str(), std::ios::binary);
	if (!ofs.is_open()) {
		HandlerUtil::generateErrorBody(req.getMethod(), res, HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	ofs.write(req.getBody().c_str(), req.getBody().length());
	ofs.close();

	res.setStatusCode(HttpStatus::CREATED);
	res.setHeader("Location", req.getPath() + "/" + filename);
	res.setBody("File uploaded successfully to " + fullUploadPath);
	res.setHeader("Content-Type", "text/plain");

	return res;
}
