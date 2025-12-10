#include "RequestResolver.hpp"
#include "../../Config/Config.hpp"
#include "../../Http/Core/HttpResponse.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Lib/Logger/ErrorLog/Logger.hpp"
#include "../../Lib/Path/Path.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include <cerrno>
#include <cstring>

namespace RequestResolver {

std::string resolvePath(const std::string &requestPath, const Config &config,
						bool skipExistenceCheck) {
	std::string bestMatchPath;
	std::string root;

	const std::map< std::string, Location > &locations = config.getLocations();
	for (std::map< std::string, Location >::const_iterator it =
			 locations.begin();
		 it != locations.end(); ++it) {
		if (requestPath.rfind(it->first, 0) == 0) {
			if (it->first.length() > bestMatchPath.length()) {
				bestMatchPath = it->first;
				root = it->second.root;
			}
		}
	}

	if (bestMatchPath.empty()) {
		return "";
	}

	std::string resolvedPath = root;
	std::string remainingPath = requestPath.substr(bestMatchPath.length());

	if (!resolvedPath.empty() &&
		resolvedPath[resolvedPath.length() - 1] != '/') {
		resolvedPath += "/";
	}
	if (!remainingPath.empty() && remainingPath[0] == '/') {
		remainingPath = remainingPath.substr(1);
	}
	resolvedPath += remainingPath;

	// 存在チェックをスキップする場合の処理
	if (skipExistenceCheck) {
		// パスを正規化する（存在チェックなし）
		resolvedPath = Path::normalize(resolvedPath);
		LOG(DEBUG) << "Resolved path: " << resolvedPath;

		// セキュリティチェックのために絶対パスを取得する
		std::string rootAbsolute = Path::getAbsolutePath(root);
		std::string fileAbsolute = Path::getAbsolutePath(resolvedPath);

		// fileAbsoluteが空の場合（ファイルが存在しない場合）、rootAbsoluteから構築する
		if (!rootAbsolute.empty()) {
			if (fileAbsolute.empty()) {
				// 存在しないファイルのために絶対パスをマニュアルで構築する
				fileAbsolute = rootAbsolute;
				if (!fileAbsolute.empty() &&
					fileAbsolute[fileAbsolute.length() - 1] != '/') {
					fileAbsolute += "/";
				}
				fileAbsolute += remainingPath;
				fileAbsolute = Path::normalize(fileAbsolute);
				resolvedPath = fileAbsolute;
			}

			// セキュリティチェック：ファイルパスがroot以下にあることを確認する
			if (fileAbsolute.rfind(rootAbsolute, 0) != 0) {
				LOG(WARNING)
					<< "Directory traversal attempt detected. Resolved path: "
					<< fileAbsolute << ", Real root: " << rootAbsolute;
				return "";
			}
		}
		return resolvedPath;
	}

	// 通常の存在チェックありの処理
	std::string originalResolvedPath = resolvedPath;
	resolvedPath = Path::getAbsolutePath(resolvedPath);
	if (resolvedPath.empty()) {
		if (errno == ENOENT) {
			LOG(DEBUG) << "Path does not exist: " << originalResolvedPath;
		} else if (errno == EACCES) {
			LOG(WARNING) << "Permission denied for path: "
						 << originalResolvedPath;
		} else {
			LOG(ERROR) << "realpath failed for path: " << originalResolvedPath
					   << " Error: " << strerror(errno);
		}
		return "";
	}

	std::string originalRoot = root;
	const std::string realRoot = Path::getAbsolutePath(root);
	if (realRoot.empty()) {
		if (errno == ENOENT) {
			LOG(DEBUG) << "Root path does not exist: " << originalRoot;
		} else if (errno == EACCES) {
			LOG(WARNING) << "Permission denied for root path: " << originalRoot;
		} else {
			LOG(ERROR) << "realpath failed for root path: " << originalRoot
					   << " Error: " << strerror(errno);
		}
		return "";
	}

	if (resolvedPath.rfind(realRoot, 0) != 0) {
		LOG(WARNING) << "Directory traversal attempt detected. Resolved path: "
					 << resolvedPath << ", Real root: " << realRoot;
		return "";
	}

	return resolvedPath;
}

bool extractCgiScript(const std::string &requestPath, const Location &loc,
					  std::string &scriptVirtual, std::string &pathInfo) {
	// Locationにマッチしているか
	const std::string &base = loc.path;
	if (requestPath.rfind(base, 0) != 0) {
		return false;
	}

	// Location部分を除いたリクエスト
	std::string requestPathWithoutBase = requestPath.substr(base.length());
	if (!requestPathWithoutBase.empty() && requestPathWithoutBase[0] == '/') {
		requestPathWithoutBase.erase(0, 1);
	}

	if (requestPathWithoutBase.empty()) {
		return false;
	}
	const std::vector< std::string > splitRequest =
		StringOps::split(requestPathWithoutBase, "/");
	int scriptIndex = 0;
	bool flag = false;
	{
		for (size_t i = 0; i < splitRequest.size(); i++) {
			const size_t dotPos = splitRequest.at(i).rfind('.');
			if (dotPos != std::string::npos) {
				const std::string ext = splitRequest.at(i).substr(dotPos);
				if (loc.cgiConf.count(ext) > 0) {
					scriptIndex = i;
					flag = true;
					break;
				}
			}
		}
	}

	if (flag == false) {
		return false;
	}

	{
		scriptVirtual = base;
		if (!scriptVirtual.empty() &&
			scriptVirtual[scriptVirtual.size() - 1] != '/') {
			scriptVirtual += "/";
		}
		for (int i = 0; i <= scriptIndex; i++) {
			if (i > 0) {
				scriptVirtual += "/";
			}
			scriptVirtual += splitRequest.at(i);
		}
	}

	{
		pathInfo = "";
		for (size_t i = scriptIndex + 1; i < splitRequest.size(); i++) {
			pathInfo += "/" + splitRequest.at(i);
		}
	}

	return true;
}

} // namespace RequestResolver
