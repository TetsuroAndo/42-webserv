#include "PostHandler.hpp"
#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"

PostHandler::PostHandler()
{
}

PostHandler::~PostHandler()
{
}

PostHandler::HttpResponse handle(const HttpRequest& req, const Config& config)
{
	HttpResponse response(SERVER_NAME);
	return response;
}
