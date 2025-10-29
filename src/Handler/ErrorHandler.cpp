#include "ErrorHandler.hpp"
#include "HandlerUtil.hpp"
#include "StaticFileHandler.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/Path/Path.hpp"
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

	const std::string &errorUri = config.getErrorPage(statusCode);
	if (!errorUri.empty()) {
		// エラーページパスを位置照合で解決する（resolvePathと同様だが存在チェックなし）
		std::string bestMatchPath;
		std::string root;

		const std::map< std::string, Location > &locations = config.getLocations();
		for (std::map< std::string, Location >::const_iterator it =
				 locations.begin();
			 it != locations.end(); ++it) {
			if (errorUri.rfind(it->first, 0) == 0) {
				if (it->first.length() > bestMatchPath.length()) {
					bestMatchPath = it->first;
					root = it->second.root;
				}
			}
		}

		if (!bestMatchPath.empty() && !root.empty()) {
			// ファイルパスを構築する： root + remainingPath
			std::string resolvedPath = root;
			std::string remainingPath = errorUri.substr(bestMatchPath.length());

			if (!resolvedPath.empty() &&
				resolvedPath[resolvedPath.length() - 1] != '/') {
				resolvedPath += "/";
			}
			if (!remainingPath.empty() && remainingPath[0] == '/') {
				remainingPath = remainingPath.substr(1);
			}
			resolvedPath += remainingPath;

			// パスを正規化する（存在チェックなし）
			resolvedPath = Path::normalize(resolvedPath);

			// セキュリティチェックのために絶対パスを取得する
			std::string rootAbsolute = Path::getAbsolutePath(root);
			std::string fileAbsolute = Path::getAbsolutePath(resolvedPath);

			// fileAbsoluteが空の場合（ファイルが存在しない場合）、rootAbsoluteから構築する
			bool isSecure = true;
			if (!rootAbsolute.empty()) {
				if (fileAbsolute.empty()) {
					// 存在しないファイルのために絶対パスを手動で構築する
					fileAbsolute = rootAbsolute;
					if (!fileAbsolute.empty() && fileAbsolute[fileAbsolute.length() - 1] != '/') {
						fileAbsolute += "/";
					}
					fileAbsolute += remainingPath;
					fileAbsolute = Path::normalize(fileAbsolute);
					resolvedPath = fileAbsolute;
				}

				// セキュリティチェック：ファイルパスがroot以下にあることを確認する
				if (fileAbsolute.rfind(rootAbsolute, 0) != 0) {
					LOG(WARNING) << "ErrorHandler: Directory traversal attempt detected"
								 << attr("file", fileAbsolute) << attr("root", rootAbsolute);
					isSecure = false;
				}
			}

			if (isSecure) {
				std::string errorContent;
				if (readErrorFile(resolvedPath, errorContent)) {
					res.setBody(errorContent);
					res.setHeader("Content-Type", "text/html");
					return res;
				}
			}
			LOG(WARNING) << "ErrorHandler: Failed to serve custom error page"
						 << attr("uri", errorUri) << attr("path", resolvedPath);
		} else {
			LOG(WARNING) << "ErrorHandler: No matching location for error page URI"
						 << attr("uri", errorUri);
		}
	}

	HandlerUtil::generateSimpleBody(ctx.req.getMethod(), res, statusCode);
	return res;
}
