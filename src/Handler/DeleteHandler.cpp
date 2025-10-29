#include "DeleteHandler.hpp"
#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/Path/Path.hpp"
#include "../Http/Resolver/RequestResolver.hpp"
#include <cstdio>
#include <cstring>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>

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

	const std::string dirPath = Path::getDirName(filePath);
	if (access(dirPath.c_str(), W_OK | X_OK) != 0) {
		return DELETE_PERMISSION_DENIED;
	}

	if (std::remove(filePath.c_str()) == 0) {
		return DELETE_SUCCESS;
	}
	return DELETE_UNKNOWN_ERROR;
}
} // namespace

DeleteHandler::DeleteHandler() {}

DeleteHandler::~DeleteHandler() {}

HttpResponse DeleteHandler::handle(PipelineContext &ctx) {
	const HttpRequest &req = ctx.req;
	HttpResponse &res = ctx.res;
	const Config &config = ctx.conf;

	LOG(INFO) << "DeleteHandler processing request"
			  << attr("method", req.getMethod()) << attr("uri", req.getPath());

	const std::string filePath =
		RequestResolver::resolvePath(req.getPath(), config);
	if (filePath.empty()) {
		LOG(WARNING) << "No matching location for DELETE request"
					 << attr("uri", req.getPath());
		res.setStatusCode(HttpStatus::NOT_FOUND);
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
		res.setStatusCode(HttpStatus::NOT_FOUND);
		break;
	case DELETE_IS_DIRECTORY:
		LOG(WARNING) << "Attempted to delete a directory"
					 << attr("path", filePath);
		res.setStatusCode(HttpStatus::FORBIDDEN);
		break;
	case DELETE_PERMISSION_DENIED:
		LOG(ERROR) << "Permission denied while deleting file"
				   << attr("path", filePath) << attr("error", strerror(errno));
		res.setStatusCode(HttpStatus::FORBIDDEN);
		break;
	case DELETE_UNKNOWN_ERROR:
		LOG(ERROR) << "Unknown error occurred while deleting file"
				   << attr("path", filePath);
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		break;
	}

	return res;
}
