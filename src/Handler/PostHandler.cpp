#include "PostHandler.hpp"

#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Http/Mime/MimeType.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/Path/Path.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Lib/Time/TimeCache.hpp"
#include "../Lib/Token/Token.hpp"
#include "HandlerUtil.hpp"

#include <cstring>
#include <iostream>
#include <sys/stat.h>

PostHandler::PostHandler() {}

PostHandler::~PostHandler() {}

namespace {
std::string removeSpaceColonCommaHyphen(const std::string &str) {
	std::string result;
	result.reserve(str.size());

	for (std::string::size_type i = 0; i < str.size(); ++i) {
		const char c = str[i];
		if (c != ' ' && c != ':' && c != ',' && c != '-') {
			result += c;
		}
	}
	return result;
}

} // namespace

HttpResponse PostHandler::handle(PipelineContext &ctx) {
	const HttpRequest &req = ctx.req;
	HttpResponse &res = ctx.res;
	const Config &config = ctx.conf;

	LOG(INFO) << "PostHandler processing request"
			  << attr("method", req.getMethod()) << attr("uri", req.getPath());

	const std::string filePath =
		HandlerUtil::resolvePath(req.getPath(), config);
	const Location &loc = config.getLocation(req.getPath());

	// ファイルパスが不正
	if (req.getPath() != loc.path) {
		LOG(INFO) << "Requested path does not match location path"
				  << attr("request_path", req.getPath())
				  << attr("location_path", loc.path);
		res.setStatusCode(HttpStatus::NOT_FOUND);
		return res;
	}

	// uploadする場所が指定されていない
	if (loc.uploadStore.empty()) {
		LOG(ERROR) << "PostHandler: Upload store is empty";
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}

	const std::string uploadStore = Path::getAbsolutePath(loc.uploadStore);
	struct stat s;
	// upload storeが存在しない
	if (stat(uploadStore.c_str(), &s) != 0 || !S_ISDIR(s.st_mode)) {
		LOG(ERROR) << "PostHandler: Upload Store \"" << uploadStore
				   << "\" is not exist or not a directory. errno: "
				   << strerror(errno);
		std::cout << uploadStore.c_str() << std::endl;
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}
	const std::string expansion =
		MimeType::getExtension(req.getHeader("Content-Type"));
	// このサーバーで処理できないMimeType
	if (expansion.empty()) {
		LOG(INFO) << "PostHandler: This Content-Type is Not Supported";
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		return res;
	}
	std::string target_filename =
		removeSpaceColonCommaHyphen(TimeCache::getGmtDate()) + "-" +
		removeSpaceColonCommaHyphen(TimeCache::getGmtTime()) + "_" +
		Token::genToken(
			8, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") +
		expansion;
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
	HandlerUtil::generateSimpleBody(req.getMethod(), res, HttpStatus::CREATED,
									"Created : " + target_filename);
	LOG(INFO) << "PostHandler : File \"" << target
			  << "\" created successfully.";
	return res;
}
