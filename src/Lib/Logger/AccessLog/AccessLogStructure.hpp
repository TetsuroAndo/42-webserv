#pragma once

#include "../../../Http/Core/HttpRequest.hpp"
#include "../../../Http/Core/HttpResponse.hpp"
#include <ctime>
#include <map>
#include <string>

struct AccessLogContext {
	time_t timestamp;
	const HttpRequest* request;
	const HttpResponse* response;
	std::string remote_addr;
	int client_port;
	std::string session_id;
};
