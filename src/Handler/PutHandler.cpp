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

	LOG(INFO) << "PostHandler processing request"
			  << attr("method", req.getMethod()) << attr("uri", req.getPath());

	const std::string filePath =
		RequestResolver::resolvePath(req.getPath(), config);
	const Location &loc = config.getLocation(req.getPath());

	// TODO: upload storeが存在しない →
	// ファイル作成をするディレクトリが存在しないに書き換える
	const std::string uploadStore = Path::getAbsolutePath(loc.uploadStore);
	struct stat s;
	if (stat(uploadStore.c_str(), &s) != 0 || !S_ISDIR(s.st_mode)) {
		LOG(ERROR) << "PostHandler: Upload Store \"" << uploadStore
				   << "\" is not exist or not a directory. errno: "
				   << strerror(errno);
		std::cout << uploadStore.c_str() << std::endl;
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}
	// TODO: リクエストからパースする
	std::string target_filename = "";
	std::string target = uploadStore + "/" + target_filename;
	std::ofstream file(target.c_str());
	// ファイル作成失敗
	if (!file) {
		LOG(ERROR) << "PostHandler: Can't create file \"" << target
				   << "\". errno: " << strerror(errno);
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	// Bodyの中身を書き込む
	file << req.getBody();
	file.close();
	DefaultPageBuilder::generateSimpleBody(req.getMethod(), res,
										   HttpStatus::CREATED,
										   "Created : " + target_filename);
	LOG(INFO) << "PutHandler : File \"" << target << "\" created successfully.";
	return res;
}
