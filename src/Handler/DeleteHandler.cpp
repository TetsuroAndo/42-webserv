#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "DeleteHandler.hpp"
#include "HandlerUtil.hpp"

#include <cstdio>
#include <sys/stat.h>

namespace {
	enum FileDeleteStatus {
		DELETE_SUCCESS,
		DELETE_NOT_FOUND,
		DELETE_IS_DIRECTORY,
		DELETE_PERMISSION_DENIED,
		DELETE_UNKNOWN_ERROR
	};

	FileDeleteStatus tryDeleteFile(const std::string& filePath) {
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

DeleteHandler::DeleteHandler() {}
DeleteHandler::~DeleteHandler() {}

HttpResponse DeleteHandler::handle(const HttpRequest &req, const Config &config) {
	HttpResponse res(SERVER_NAME);

	if (req.getMethod() != "DELETE") {
		HandlerUtil::generateErrorBody(res, HttpStatus::METHOD_NOT_ALLOWED);
		return res;
	}

	const std::string filePath = HandlerUtil::resolvePath(req.getPath(), config);
	if (filePath.empty()) {
		HandlerUtil::generateErrorBody(res, HttpStatus::NOT_FOUND);
		return res;
	}

	const FileDeleteStatus deleteStatus = tryDeleteFile(filePath);

	switch (deleteStatus) {
		case DELETE_SUCCESS:
			res.setStatusCode(HttpStatus::NO_CONTENT);
			break;
		case DELETE_NOT_FOUND:
			HandlerUtil::generateErrorBody(res, HttpStatus::NOT_FOUND);
			break;
		case DELETE_IS_DIRECTORY:
			HandlerUtil::generateErrorBody(res, HttpStatus::FORBIDDEN);
			break;
		case DELETE_PERMISSION_DENIED:
			HandlerUtil::generateErrorBody(res, HttpStatus::FORBIDDEN);
			break;
		case DELETE_UNKNOWN_ERROR:
			HandlerUtil::generateErrorBody(res, HttpStatus::INTERNAL_SERVER_ERROR);
			break;
	}

	return res;
}
