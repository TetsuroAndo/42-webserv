#pragma once

#include "../../../Http/Core/HttpRequest.hpp"
#include "../../../Http/Core/HttpResponse.hpp"
#include <map>
#include <string>

struct AccessLogContext {
	std::string utcTimestamp;
	std::string isoTimestamp;
	const HttpRequest *request;
	const HttpResponse *response;
	std::string remote_addr;
	int client_port;
	std::string session_id;
};
