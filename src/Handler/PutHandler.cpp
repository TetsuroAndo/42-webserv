#include "PutHandler.hpp"
#include "../Config/Config.hpp"
#include "../Http/Builder/DefaultPageBuilder.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Http/Mime/MimeType.hpp"
#include "../Http/Resolver/RequestResolver.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/Path/Path.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Lib/Time/TimeCache.hpp"
#include "../Lib/Token/Token.hpp"
#include <cstring>
#include <iostream>
#include <sys/stat.h>

PutHandler::PutHandler() {}

PutHandler::~PutHandler() {}

HttpResponse PutHandler::handle(PipelineContext &ctx) {
	const HttpRequest &req = ctx.req;
	HttpResponse &res = ctx.res;
	const Config &config = ctx.conf;

	LOG(INFO) << "PutHandler processing request"
			  << attr("method", req.getMethod()) << attr("uri", req.getPath());

	const std::string filePath =
		RequestResolver::resolvePath(req.getPath(), config);
	const Location &loc = config.getLocation(req.getPath());

	std::string pathWithoutBase = req.getPath().substr(loc.path.length());

	if (pathWithoutBase.empty() == false && pathWithoutBase[0] == '/') {
		pathWithoutBase = pathWithoutBase.substr(1);
	}

	std::vector< std::string > splitPath =
		StringOps::split(pathWithoutBase, "/");
	if (splitPath.empty()) {
		LOG(ERROR) << "PutHandler: No target file specified in request path.";
		res.setStatusCode(HttpStatus::BAD_REQUEST);
		return res;
	}
	std::string uploadStore = loc.root;
	if (uploadStore.empty() == false &&
		uploadStore[uploadStore.length() - 1] == '/') {
		uploadStore = uploadStore.substr(0, uploadStore.length() - 1);
	}
	{
		if (0 < splitPath.size()) {
			for (size_t i = 0; i < splitPath.size() - 1; ++i) {
				uploadStore += "/" + splitPath[i];
			}
		}
	}

	struct stat directoryStat;
	if (stat(uploadStore.c_str(), &directoryStat) != 0 ||
		!S_ISDIR(directoryStat.st_mode)) {
		LOG(ERROR) << "PutHandler: Upload Store \"" << uploadStore
				   << "\" is not exist or not a directory. errno: "
				   << strerror(errno);
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	std::string target_filename = splitPath[splitPath.size() - 1];
	std::string target = uploadStore + "/" + target_filename;

	int status = HttpStatus::CREATED;
	struct stat fileStat;
	if (stat(target.c_str(), &fileStat) == 0) {
		if (S_ISREG(fileStat.st_mode)) {
			status = HttpStatus::NO_CONTENT;
		}
	}

	std::ofstream file(target.c_str());
	// ファイル作成失敗
	if (!file) {
		LOG(ERROR) << "PutHandler: Can't create file \"" << target
				   << "\". errno: " << strerror(errno);
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	// Bodyの中身を書き込む
	file << req.getBody();
	file.close();
	DefaultPageBuilder::generateSimpleBody(req.getMethod(), res, status,
										   "Created : " + target_filename);
	ctx.res.appendHeader("Content-Location", req.getPath());
	LOG(INFO) << "PutHandler : File \"" << target << "\" created successfully.";
	return res;
}
