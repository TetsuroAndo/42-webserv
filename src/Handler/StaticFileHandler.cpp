#include "StaticFileHandler.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Http/Mime/MimeType.hpp"
#include "HandlerUtil.hpp"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <vector>

namespace {
	enum FileReadStatus {
		FILE_READ_SUCCESS,
		FILE_READ_NOT_FOUND,
		FILE_READ_IS_DIRECTORY,
		FILE_READ_FORBIDDEN,
		FILE_READ_ERROR
	};

	FileReadStatus tryReadFile(const std::string& filePath, std::string& outContent, const struct stat& fileStat) {

		std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
		if (!file) {
			return FILE_READ_FORBIDDEN;
		}

		std::streampos fileSize = fileStat.st_size;

		outContent.resize(fileSize);
		file.read(&outContent[0], fileSize);
		file.close();

		return FILE_READ_SUCCESS;
	}
} // namespace

StaticFileHandler::StaticFileHandler() {}
StaticFileHandler::~StaticFileHandler() {}

// Helper to generate an HTML page for directory listing
void generateDirectoryListing(HttpResponse &res,
					 const std::string &directoryPath,
					 const std::string &requestPath) {
	DIR *dir = opendir(directoryPath.c_str());
	if (!dir) {
		HandlerUtil::generateErrorBody(res, HttpStatus::INTERNAL_SERVER_ERROR);
		return;
	}

	std::string htmlContent;
	htmlContent += "<html><head><title>Index of ";
	htmlContent += requestPath;
	htmlContent += "</title></head><body><h1>Index of ";
	htmlContent += requestPath;
	htmlContent += "</h1><hr><pre>";

	std::vector<std::string> files;
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		files.push_back(entry->d_name);
	}
	closedir(dir);
	std::sort(files.begin(), files.end());

	for (std::vector<std::string>::const_iterator it = files.begin();
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

	res.setStatusCode(HttpStatus::OK);
	res.setHeader("Content-Type", "text/html");
	res.setBody(htmlContent);
}

HttpResponse StaticFileHandler::handle(const HttpRequest &req, const Config &config) {
	HttpResponse res(SERVER_NAME);

	if (req.getMethod() != "GET") {
		HandlerUtil::generateErrorBody(res, HttpStatus::METHOD_NOT_ALLOWED);
		return res;
	}

	std::string filePath = HandlerUtil::resolvePath(req.getPath(), config);
	if (filePath.empty()) {
		HandlerUtil::generateErrorBody(res, HttpStatus::NOT_FOUND);
		return res;
	}

	struct stat pathStat;
	if (stat(filePath.c_str(), &pathStat) != 0) {
		HandlerUtil::generateErrorBody(res, HttpStatus::NOT_FOUND);
		return res;
	}

	if (S_ISDIR(pathStat.st_mode)) {
		std::string indexPath = filePath + "/" + config.getLocation("/").indexFile;
		struct stat indexStat;
		if (stat(indexPath.c_str(), &indexStat) == 0 &&
			S_ISREG(indexStat.st_mode)) {
			filePath = indexPath;
			pathStat = indexStat;
		} else {
			if (config.getLocation("/").autoindex) {
				generateDirectoryListing(res, filePath, req.getPath());
			} else {
				HandlerUtil::generateErrorBody(res, HttpStatus::FORBIDDEN);
			}
			return res;
		}
	}

	if (S_ISREG(pathStat.st_mode)) {
		std::string fileContent;
		FileReadStatus readStatus = tryReadFile(filePath, fileContent, pathStat);

		switch (readStatus) {
			case FILE_READ_SUCCESS:
				res.setStatusCode(HttpStatus::OK);
				res.setHeader("Content-Type", MimeType::getMimeType(filePath));
				if (req.getMethod() == "GET") {
					res.setBody(fileContent);
				}
				break;
			case FILE_READ_NOT_FOUND:
				HandlerUtil::generateErrorBody(res, HttpStatus::NOT_FOUND);
				break;
			case FILE_READ_IS_DIRECTORY:
				HandlerUtil::generateErrorBody(res, HttpStatus::FORBIDDEN);
				break;
			case FILE_READ_FORBIDDEN:
				HandlerUtil::generateErrorBody(res, HttpStatus::FORBIDDEN);
				break;
			case FILE_READ_ERROR:
				HandlerUtil::generateErrorBody(res, HttpStatus::INTERNAL_SERVER_ERROR);
				break;
		}
	} else {
		HandlerUtil::generateErrorBody(res, HttpStatus::FORBIDDEN);
	}

	return res;
}
