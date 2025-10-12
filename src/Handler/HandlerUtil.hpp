#pragma once

#include <string>

class HttpResponse;
class Config;

namespace HandlerUtil
{
    std::string resolvePath(const std::string& requestPath, const Config& config);

    void generateSimpleBody(const std::string& method, HttpResponse& res, int code, const std::string &description = "");

    std::string getRealPath(const char* path);
} // namespace HandlerUtil
