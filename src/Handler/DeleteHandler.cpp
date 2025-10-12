#include "DeleteHandler.hpp"
#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Info/App.hpp"
#include "../Lib/Logger/Log.hpp"
#include "HandlerUtil.hpp"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace {
enum FileDeleteStatus {
	DELETE_SUCCESS,
	DELETE_NOT_FOUND,
	DELETE_IS_DIRECTORY,
	DELETE_PERMISSION_DENIED,
	DELETE_UNKNOWN_ERROR
};

FileDeleteStatus tryDeleteFile(const std::string &filePath) {
	struct stat pathStat;
	if (stat(filePath.c_str(), &pathStat) != 0) {
		return DELETE_NOT_FOUND;
	}

	if (S_ISDIR(pathStat.st_mode)) {
		return DELETE_IS_DIRECTORY;
	}

	if (std::remove(filePath.c_str()) == 0) {
		return DELETE_SUCCESS;
	}
	return DELETE_PERMISSION_DENIED;
}
} // namespace

DeleteHandler::DeleteHandler() {
}

DeleteHandler::~DeleteHandler() {
}

HttpResponse DeleteHandler::handle(const HttpRequest &req,
                                   const Config &config) {
	HttpResponse res(SERVER_NAME);
	LOG(INFO) << "DeleteHandler processing request"
			  << attr("method", req.getMethod())
			  << attr("uri", req.getPath());

	const std::string filePath =
		HandlerUtil::resolvePath(req.getPath(), config);
	if (filePath.empty()) {
		LOG(WARNING) << "No matching location for DELETE request"
					 << attr("uri", req.getPath());
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::NOT_FOUND);
		return res;
	}

	const FileDeleteStatus deleteStatus = tryDeleteFile(filePath);

	switch (deleteStatus) {
	case DELETE_SUCCESS:
		LOG(INFO) << "File deleted successfully" << attr("path", filePath);
		res.setStatusCode(HttpStatus::NO_CONTENT);
		break;
	case DELETE_NOT_FOUND:
		LOG(WARNING) << "File not found for deletion" << attr("path", filePath);
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::NOT_FOUND);
		break;
	case DELETE_IS_DIRECTORY:
		LOG(WARNING) << "Attempted to delete a directory"
					 << attr("path", filePath);
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::FORBIDDEN);
		break;
	case DELETE_PERMISSION_DENIED:
		LOG(ERROR) << "Permission denied while deleting file"
				   << attr("path", filePath)
				   << attr("error", strerror(errno));
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::FORBIDDEN);
		break;
	case DELETE_UNKNOWN_ERROR:
		LOG(ERROR) << "Unknown error occurred while deleting file"
				   << attr("path", filePath);
		HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::INTERNAL_SERVER_ERROR);
		break;
	}

	return res;
}
