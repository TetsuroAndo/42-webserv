#include "ErrorHandler.hpp"
#include "../Http/Builder/DefaultPageBuilder.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Http/Resolver/RequestResolver.hpp"
#include "../Lib/Logger/Log.hpp"
#include <fstream>
#include <sys/stat.h>

ErrorHandler::ErrorHandler() {}
ErrorHandler::~ErrorHandler() {}

namespace {

/**
 * @brief エラーファイルを読み込む
 * @param filePath エラーファイルのパス
 * @param outContent エラーファイルの内容
 * @return 成功時true、失敗時false
 */
bool readErrorFile(const std::string &filePath, std::string &outContent) {
	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
	if (!file) {
		LOG(ERROR) << "ErrorHandler: Failed to open error file"
				   << attr("path", filePath);
		return false;
	}

	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) != 0) {
		LOG(ERROR) << "ErrorHandler: Failed to stat error file"
				   << attr("path", filePath);
		return false;
	}

	const std::streampos fileSize = fileStat.st_size;
	outContent.resize(fileSize);
	file.read(&outContent[0], fileSize);
	if (!file) {
		LOG(ERROR) << "ErrorHandler: Failed to read error file"
				   << attr("path", filePath);
		return false;
	}
	file.close();
	return true;
}
} // namespace

HttpResponse ErrorHandler::handle(PipelineContext &ctx) {
	const Config &config = ctx.conf;
	HttpResponse &res = ctx.res;
	const int statusCode = res.getStatusCode();

	std::string tmp;
	if (res.isDirectoryResponse()) {
		const Location location = config.getLocation(ctx.req.getPath());
		tmp = location.path + "/" + location.directoryError;
	}
	if (tmp.empty())
		tmp = config.getErrorPage(statusCode);
	const std::string &errorUri = tmp;
	if (!errorUri.empty()) {
		// エラーページパスを位置照合で解決する（存在チェックなし）
		const std::string resolvedPath =
			RequestResolver::resolvePath(errorUri, config, true);
		if (!resolvedPath.empty()) {
			std::string errorContent;
			if (readErrorFile(resolvedPath, errorContent)) {
				res.setBody(errorContent);
				res.setHeader("Content-Type", "text/html");
				return res;
			}
			LOG(WARNING) << "ErrorHandler: Failed to serve custom error page"
						 << attr("uri", errorUri) << attr("path", resolvedPath);
		} else {
			LOG(WARNING)
				<< "ErrorHandler: No matching location for error page URI"
				<< attr("uri", errorUri);
		}
	}

	DefaultPageBuilder::generateSimpleBody(ctx.req.getMethod(), res,
										   statusCode);
	return res;
}
