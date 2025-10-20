#include "StaticFileHandler.hpp"
#include "../Config/PerformanceConfig.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Http/Mime/MimeType.hpp"
#include "../Lib/Logger/Log.hpp"
#include "HandlerUtil.hpp"
#include <algorithm>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

namespace {

enum FileReadStatus { FILE_READ_SUCCESS, FILE_READ_ERROR, FILE_READ_FORBIDDEN };

FileReadStatus tryReadFile(const std::string &filePath, std::string &outContent,
						   const struct stat &fileStat) {
	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);

	if (!file) {
		const int err = errno;
		switch (err) {
		case EACCES:
		case EPERM:
			LOG(WARNING) << "Permission denied while opening file"
						 << attr("path", filePath)
						 << attr("error", strerror(err));
			return FILE_READ_FORBIDDEN;
		case ENOENT:
		case ENOTDIR:
			LOG(WARNING) << "File not found or invalid path during open"
						 << attr("path", filePath)
						 << attr("error", strerror(err));
			break;

		default:
			LOG(ERROR) << "OS-level error while opening file"
					   << attr("path", filePath) << attr("errno", err)
					   << attr("error", strerror(err));
			break;
		}
		return FILE_READ_ERROR;
	}
	const std::streampos fileSize = fileStat.st_size;
	outContent.resize(fileSize);
	file.read(&outContent[0], fileSize);
	if (!file) {
		const int err = errno;
		LOG(ERROR) << "Error while reading file" << attr("path", filePath)
				   << attr("errno", err) << attr("error", strerror(err));
		return FILE_READ_ERROR;
	}

	file.close();
	return FILE_READ_SUCCESS;
}

} // namespace

StaticFileHandler::StaticFileHandler() {}

StaticFileHandler::~StaticFileHandler() {}

void StaticFileHandler::generateDirectoryListing(
	PipelineContext &ctx, const std::string &directoryPath,
	const std::string &requestPath) {
	HttpResponse &res = ctx.res;
	const HttpRequest &req = ctx.req;
	DIR *dir = opendir(directoryPath.c_str());
	if (!dir) {
		LOG(ERROR) << "Failed to open directory for listing"
				   << attr("path", directoryPath)
				   << attr("error", strerror(errno));
		HandlerUtil::generateSimpleBody(req.getMethod(), res,
										HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}

	std::string htmlContent;
	htmlContent += "<html><head><title>Index of ";
	htmlContent += requestPath;
	htmlContent += "</title></head><body><h1>Index of ";
	htmlContent += requestPath;
	htmlContent += "</h1><hr><pre>";

	std::vector< std::string > files;
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		files.push_back(entry->d_name);
	}
	closedir(dir);
	std::sort(files.begin(), files.end());

	for (std::vector< std::string >::const_iterator it = files.begin();
		 it != files.end(); ++it) {
		std::string name = *it;
		std::string linkPath = requestPath;
		if (linkPath.empty() || linkPath[linkPath.length() - 1] != '/') {
			linkPath += "/";
		}
		linkPath += name;
		htmlContent += "<a href=\"" + linkPath + "\">" + name + "</a>\n";
	}

	htmlContent += "</pre><hr></body></html>";

	res.statusCode = HttpStatus::OK;
	res.headers["Content-Type"] = "text/html";
	if (req.getMethod() == "GET") {
		res.body = htmlContent;
	} else {
		res.body = "";
	}
}

HttpResponse StaticFileHandler::handle(PipelineContext &ctx) {
	const HttpRequest &req = ctx.req;
	HttpResponse &res = ctx.res;
	const Config &config = ctx.conf;
	LOG(INFO) << "StaticFileHandler processing request"
			  << attr("method", req.getMethod()) << attr("uri", req.getPath());

	std::string filePath = HandlerUtil::resolvePath(req.getPath(), config);
	if (filePath.empty()) {
		LOG(WARNING) << "No matching location found for URI"
					 << attr("uri", req.getPath());
		HandlerUtil::generateSimpleBody(req.getMethod(), res,
										HttpStatus::NOT_FOUND);
		return res;
	}

	struct stat pathStat;
	if (stat(filePath.c_str(), &pathStat) != 0) {
		LOG(WARNING) << "File or directory not found" << attr("path", filePath)
					 << attr("error", strerror(errno));
		HandlerUtil::generateSimpleBody(req.getMethod(), res,
										HttpStatus::NOT_FOUND);
		return res;
	}

	if (S_ISDIR(pathStat.st_mode)) {
		const Location &loc = config.getLocation(req.getPath());
		std::string indexPath = filePath + "/" + loc.indexFile;
		struct stat indexStat;
		if (stat(indexPath.c_str(), &indexStat) == 0 &&
			S_ISREG(indexStat.st_mode)) {
			LOG(DEBUG) << "Serving index file" << attr("path", indexPath);
			filePath = indexPath;
			pathStat = indexStat;
		} else {
			if (loc.autoindex) {
				LOG(INFO) << "Generating directory listing for"
						  << attr("path", filePath);
				generateDirectoryListing(ctx, filePath, req.getPath());
				if (req.getMethod() != "GET") {
					std::ostringstream oss;
					oss << res.body.length();
					res.headers["Content-Length"] = oss.str();
					res.body = "";
				}
			} else {
				LOG(WARNING) << "Directory listing is disabled for"
							 << attr("path", filePath);
				HandlerUtil::generateSimpleBody(req.getMethod(), res,
												HttpStatus::FORBIDDEN);
			}
			return res;
		}
	}

	if (S_ISREG(pathStat.st_mode)) {
		std::string fileContent;
		FileReadStatus readStatus =
			tryReadFile(filePath, fileContent, pathStat);

		switch (readStatus) {
		case FILE_READ_SUCCESS:
			res.statusCode = HttpStatus::OK;
			res.headers["Content-Type"] = MimeType::getMimeType(filePath);
			if (req.getMethod() == "GET") {
				res.body = fileContent;
			} else {
				std::ostringstream oss;
				oss << pathStat.st_size;
				res.headers["Content-Length"] = oss.str();
			}
			LOG(DEBUG) << "Successfully served file" << attr("path", filePath);
			break;
		case FILE_READ_FORBIDDEN:
			HandlerUtil::generateSimpleBody(req.getMethod(), res,
											HttpStatus::FORBIDDEN);
			break;
		case FILE_READ_ERROR:
			HandlerUtil::generateSimpleBody(req.getMethod(), res,
											HttpStatus::INTERNAL_SERVER_ERROR);
			break;
		}
	} else {
		LOG(WARNING) << "Requested path is not a regular file or directory"
					 << attr("path", filePath);
		HandlerUtil::generateSimpleBody(req.getMethod(), res,
										HttpStatus::INTERNAL_SERVER_ERROR);
	}
	return res;
}
