#include "PostHandler.hpp"
#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "HandlerUtil.hpp"

#include <iostream>

PostHandler::PostHandler()
{
}

PostHandler::~PostHandler()
{
}

HttpResponse PostHandler::handle(const HttpRequest& req, const Config& config)
{
	HttpResponse response(SERVER_NAME);
	std::string filePath = HandlerUtil::resolvePath(req.getPath(), config);
	std::cout << "Called!: " << filePath << std::endl;
	return response;
}
