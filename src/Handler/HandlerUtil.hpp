#pragma once

#include <string>

class HttpResponse;
class Config;

namespace HandlerUtil {
std::string resolvePath(const std::string &requestPath, const Config &config);

void generateErrorBody(HttpResponse &res, int code);

std::string toString(int value);

std::string getRealPath(const char *path);
} // namespace HandlerUtil
