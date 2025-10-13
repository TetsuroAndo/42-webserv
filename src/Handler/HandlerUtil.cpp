#include "HandlerUtil.hpp"

#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "../Lib/Path/Path.hpp"
#include "../Lib/Logger/ErrorLog/Logger.hpp"
#include <cerrno>
#include <cstring>

namespace HandlerUtil {

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
		body += "<p>";
		body += description;
		body += "</p>";
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

    std::string originalResolvedPath = resolvedPath;
	resolvedPath = Path::getAbsolutePath(resolvedPath);
	if (resolvedPath.empty()) {
        if (errno == ENOENT) {
            LOG(DEBUG) << "Path does not exist: " << originalResolvedPath;
        } else if (errno == EACCES) {
            LOG(WARNING) << "Permission denied for path: " << originalResolvedPath;
        } else {
            LOG(ERROR) << "realpath failed for path: " << originalResolvedPath << " Error: " << strerror(errno);
        }
		return "";
	}

    std::string originalRoot = root;
	const std::string realRoot = Path::getAbsolutePath(root);
	if (realRoot.empty()) {
        if (errno == ENOENT) {
            LOG(DEBUG) << "Root path does not exist: " << originalRoot;
        }
        else if (errno == EACCES) {
            LOG(WARNING) << "Permission denied for root path: " << originalRoot;
        } else {
            LOG(ERROR) << "realpath failed for root path: " << originalRoot << " Error: " << strerror(errno);
        }
		return "";
	}

	if (resolvedPath.rfind(realRoot, 0) != 0) {
        LOG(WARNING) << "Directory traversal attempt detected. Resolved path: " << resolvedPath << ", Real root: " << realRoot;
		return "";
	}

	return resolvedPath;
}
} // namespace HandlerUtil
