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
std::string removeSpaceCoronComma(const std::string &str) {
	std::string result;
	result.reserve(str.size());

	for (std::string::size_type i = 0; i < str.size(); ++i) {
		const char c = str[i];
		if (c != ' ' && c != ':' && c != ',') {
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
	// 工事現場はこちらです。
	std::cout << "Called!: \n"
		<< "    req head : "
		<< (req.getHeader("Content-Type").empty()
			    ? "empty"
			    : req.getHeader("Content-Type"))
		<< std::endl
		<< "    req body : "
		<< (req.getBody().empty() ? 0 : req.getBody().size()) << std::endl
		<< "    req name : "
		<< (req.getPath().empty() ? "empty" : req.getPath()) << std::endl;

	//  curl -X POST http://127.0.0.1:8080/ \
	// -H "Content-Type: image/png" \
	// --data-binary "@./tmp/img.png"

	// 上記のコマンドを、@./tmp/img.pngを用意した状態でdefault.yamlでサーバーを起動すると動く

	const Location &loc = config.getLocation(req.getPath());
	const std::string uploadStore = loc.uploadStore;
	// uploadする場所が指定されていない
	if (loc.uploadStore.empty()) {
		std::cout << "-1" << std::endl;
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}
	struct stat s;
	// upload storeが存在しない
	if (stat(uploadStore.c_str(), &s) != 0 || !S_ISDIR(s.st_mode)) {
		std::cout << "0" << std::endl;
		std::cout << uploadStore.c_str() << std::endl;
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}
	const std::string expansion =
		MimeType::getExtension(req.getHeader("Content-Type"));
	// このサーバーで処理できないMimeType
	if (expansion.empty()) {
		std::cout << "2" << std::endl;
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}
	std::string target = uploadStore + "/" + Token::genToken(32) + "_" +
	                     removeSpaceCoronComma(TimeCache::getCurrentTime()) +
	                     expansion;
	std::ofstream file(target.c_str());
	// ファイル作成失敗
	if (!file) {
		std::cout << "3" << std::endl;
		HandlerUtil::generateErrorBody(req.getMethod(), response,
		                               HttpStatus::INTERNAL_SERVER_ERROR);
		return response;
	}
	// Bodyの中身を書き込む
	file << req.getBody();
	file.close();
	HandlerUtil::generateErrorBody(req.getMethod(), response,
	                               HttpStatus::CREATED);
	return response;
}
