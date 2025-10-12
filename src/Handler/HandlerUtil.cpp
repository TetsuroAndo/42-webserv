#include "HandlerUtil.hpp"

#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/StringOps/StringOps.hpp"

#include <cstdio>
#include <cstdlib>

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

void generateSimpleBody(const std::string &method, HttpResponse &res,
                        const int code,  const std::string &description) {
	res.setStatusCode(code);
	const std::string &reason = HttpStatus::getReason(code);
	std::string body;
	body += "<html><head><title>";
	body += StringOps::toString(code);
	body += " ";
	body += reason;
	body += "</title></head><body><h1>";
	body += StringOps::toString(code);
	body += " ";
	body += reason;
	body += "</h1>";
	if (description.empty() == false) {
		body += "<h2>";
		body += description;
		body += "</h2>";
	}
	body += "</body></html>";
	res.setBody(body);
	res.setHeader("Content-Type", "text/html");
	if (method == "HEAD") {
		res.setBody("");
	} else {
		res.setBody(body);
	}
}

std::string resolvePath(const std::string &requestPath, const Config &config) {
	std::string bestMatchPath;
	std::string root;

	const std::map<std::string, Location> &locations = config.getLocations();
	for (std::map<std::string, Location>::const_iterator it = locations.begin();
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

	resolvedPath = getRealPath(resolvedPath.c_str());
	if (resolvedPath.empty()) {
		return "";
	}

	const std::string realRoot = getRealPath(root.c_str());
	if (realRoot.empty()) {
		return "";
	}

	if (resolvedPath.rfind(realRoot, 0) != 0) {
		return "";
	}

	return resolvedPath;
}
} // namespace HandlerUtil
