#include "Path.hpp"
#include <cstdlib>
#include <sstream>
#include <vector>
#include <limits.h>

namespace Path {

std::string getAbsolutePath(const std::string &path) {
    char buf[PATH_MAX];

    if (realpath(path.c_str(), buf) == NULL) {
        return "";
    }

    return std::string(buf);
}

std::string normalize(const std::string &path) {
	if (path.empty()) {
		return ".";
	}

	bool isAbsolute = (path[0] == '/');
	std::vector< std::string > components;
	std::string component;
	std::stringstream ss(path);

	while (std::getline(ss, component, '/')) {
		if (component.empty() || component == ".") {
			continue;
		}
		if (component == "..") {
			if (!components.empty() && components.back() != "..") {
				components.pop_back();
			} else {
				if (!isAbsolute) {
					components.push_back("..");
				}
			}
		} else {
			components.push_back(component);
		}
	}

	if (components.empty()) {
		return isAbsolute ? "/" : ".";
	}

	std::string result;
	if (isAbsolute) {
		result = "/";
	}

	for (size_t i = 0; i < components.size(); ++i) {
		result += components[i];
		if (i < components.size() - 1) {
			result += "/";
		}
	}

	return result;
}

std::string getDirName(const std::string &path) {
	if (path.empty())
		return "";
	std::string::size_type pos = path.find_last_of('/');
	if (pos == std::string::npos)
		return ".";
	if (pos == 0)
		return "/";
	return path.substr(0, pos);
}

} // namespace Path
