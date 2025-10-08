#pragma once

#include "../Config/Config.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include <string>
#include <vector>

class CgiEnvironmentBuilder {
public:
	static std::vector<std::string> build(const HttpRequest& req,
										  const Location& locConf,
										  const std::string& scriptPath);
};
