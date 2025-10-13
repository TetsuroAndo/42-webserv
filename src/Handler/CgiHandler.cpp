#include "CgiHandler.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Info/App.hpp"
#include "../Lib/Logger/Log.hpp"
#include "HandlerUtil.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

HttpResponse CgiHandler::handle(const HttpRequest &req, HttpResponse &res, const Config &config) {
	LOG(INFO) << "CgiHandler processing request" << attr("method", req.getMethod())
			  << attr("uri", req.getPath());

	if (req.getMethod() != "POST") {
		LOG(WARNING) << "Method not allowed for CgiHandler"
					 << attr("method", req.getMethod());
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::METHOD_NOT_ALLOWED);
		return res;
	}

	const Location &loc = config.getLocation(req.getPath());
	std::string uploadStore = loc.uploadStore;

	if (uploadStore.empty()) {
		LOG(ERROR) << "Upload store is not configured for this location"
				   << attr("uri", req.getPath());
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	struct stat s;
	if (stat(uploadStore.c_str(), &s) != 0 || !S_ISDIR(s.st_mode)) {
		LOG(ERROR) << "Upload store path is not a valid directory"
				   << attr("path", uploadStore);
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	std::string filename = "uploaded_file.bin";
	std::string fullUploadPath = uploadStore + "/" + filename;

	std::ofstream ofs(fullUploadPath.c_str(), std::ios::binary);
	if (!ofs.is_open()) {
		LOG(ERROR) << "Failed to open file for writing" << attr("path", fullUploadPath)
				   << attr("error", strerror(errno));
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	ofs.write(req.getBody().c_str(), req.getBody().length());
	ofs.close();

	LOG(INFO) << "File uploaded successfully" << attr("path", fullUploadPath)
			  << attr("size", req.getBody().length());

	res.setStatusCode(HttpStatus::CREATED);
	res.setHeader("Location", req.getPath() + "/" + filename);
	res.setBody("File uploaded successfully to " + fullUploadPath);
	res.setHeader("Content-Type", "text/plain");

	return res;
}
