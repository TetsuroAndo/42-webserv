#include "HandlerUtil.hpp"

#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace HandlerUtil {

std::string getRealPath(const char *path) {
	char *realPathPtr = realpath(path, NULL);
	if (realPathPtr == NULL) {
		return "";
	}
	std::string realPath(realPathPtr);
	free(realPathPtr);
	return realPath;
}

std::string toString(int value) {
	char buffer[32];
	std::sprintf(buffer, "%d", value);
	return std::string(buffer);
}

void generateErrorBody(HttpResponse &res, int code) {
	res.setStatusCode(code);
	const std::string &reason = HttpStatus::getReason(code);
	std::string body;
	body += "<html><head><title>";
	body += HandlerUtil::toString(code);
	body += " ";
	body += reason;
	body += "</title></head><body><h1>";
	body += HandlerUtil::toString(code);
	body += " ";
	body += reason;
	body += "</h1></body></html>";
	res.setBody(body);
	res.setHeader("Content-Type", "text/html");
}

std::string resolvePath(const std::string &requestPath, const Config &config) {
	std::string bestMatchPath;
	std::string root;

	const std::vector<Location> &locations = config.getLocations();
	for (std::vector<Location>::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		if (requestPath.rfind(it->path, 0) == 0) {
			if (it->path.length() > bestMatchPath.length()) {
				bestMatchPath = it->path;
				root = it->root;
			}
		}
	}

	if (bestMatchPath.empty()) {
		return "";
	}

	std::string resolvedPath = root;
	std::string remainingPath = requestPath.substr(bestMatchPath.length());

	if (!resolvedPath.empty() && resolvedPath[resolvedPath.length() - 1] != '/') {
		resolvedPath += "/";
	}
	if (!remainingPath.empty() && remainingPath[0] == '/') {
		remainingPath = remainingPath.substr(1);
	}
	resolvedPath += remainingPath;

	resolvedPath = HandlerUtil::getRealPath(resolvedPath.c_str());
	if (resolvedPath.empty()) {
		return "";
	}

	std::string realRoot = HandlerUtil::getRealPath(root.c_str());
	if (realRoot.empty()) {
		return "";
	}

	if (resolvedPath.rfind(realRoot, 0) != 0) {
		return "";
	}

	return resolvedPath;
}

} // namespace HandlerUtil
