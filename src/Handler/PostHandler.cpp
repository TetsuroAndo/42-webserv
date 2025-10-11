#include "PostHandler.hpp"

#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Http/Mime/MimeType.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/Time/TimeCache.hpp"
#include "../Lib/Token/Token.hpp"
#include "HandlerUtil.hpp"

#include <cstring>
#include <iostream>
#include <sys/stat.h>

PostHandler::PostHandler() {
}

PostHandler::~PostHandler() {
}

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

HttpResponse PostHandler::handle(const HttpRequest &req, const Config &config) {
	HttpResponse response(SERVER_NAME);
	LOG(INFO) << "PostHandler processing request"
			  << attr("method", req.getMethod()) << attr("uri", req.getPath());

	const std::string filePath =
		HandlerUtil::resolvePath(req.getPath(), config);
	const Location &loc = config.getLocation(req.getPath());

	// ファイルパスが不正
	if (req.getPath() != loc.path) {
		LOG(INFO) << "Requested path does not match actual path";
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::NOT_FOUND);
		return response;
	}

	const std::string uploadStore = loc.uploadStore;

	// uploadする場所が指定されていない
	if (loc.uploadStore.empty()) {
		LOG(ERROR) << "Upload store is empty";
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}
	struct stat s;
	// upload storeが存在しない
	if (stat(uploadStore.c_str(), &s) != 0 || !S_ISDIR(s.st_mode)) {
		LOG(ERROR) << "PostHandler : Upload Store \"" << uploadStore <<
 "\" is not exist.";
		std::cout << uploadStore.c_str() << std::endl;
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}
	const std::string expansion =
		MimeType::getExtension(req.getHeader("Content-Type"));
	// このサーバーで処理できないMimeType
	if (expansion.empty()) {
		LOG(INFO) << "PostHandler : This Content-Type is Not Supported";
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}
	std::string target = uploadStore + "/" +
	                     removeSpaceColonCommaHyphen(TimeCache::getGmtDate()) +
	                     "-" + removeSpaceColonCommaHyphen(
		                     TimeCache::getGmtTime()) +
	                     "_" +
	                     Token::genToken(
		                     8,
		                     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz")
	                     +
	                     expansion;
	std::ofstream file(target.c_str());
	// ファイル作成失敗
	if (!file) {
		LOG(ERROR) << "PostHandler : Can't create file";
		HandlerUtil::generateErrorBody(req.getMethod(), response,
									   HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}

	// Bodyが大きすぎる
	if (config.getMaxRequestBodySize() < req.getBody().size()) {
		LOG(ERROR) << "PostHandler : Payload too large";
		HandlerUtil::generateErrorBody(req.getMethod(), response,
									   HttpStatus::PAYLOAD_TOO_LARGE);
		return response;
	}

	// Bodyの中身を書き込む
	file << req.getBody();
	file.close();
	HandlerUtil::generateErrorBody(req.getMethod(), response,
	                               HttpStatus::CREATED);
	LOG(INFO) << "PostHandler : File \"" << target <<
 "\" created successfully.";
	return response;
}
