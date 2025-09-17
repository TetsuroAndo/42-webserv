#pragma once

#include <string>
#include <map>

namespace HttpStatus {

	static const std::map<int, std::string> statusReasons = {
		{100, "Continue"},
		// Success
		{200, "OK"},
		{201, "Created"},
		{202, "Accepted"},
		{204, "No Content"},
		// Redirection
		{301, "Moved Permanently"},
		{302, "Found"},
		{303, "See Other"},
		// Client Error
		{400, "Bad Request"},
		{401, "Unauthorized"},
		{403, "Forbidden"},
		{404, "Not Found"},
		{405, "Method Not Allowed"},
		{408, "Request Timeout"},
		{413, "Payload Too Large"},
		{414, "URI Too Long"},
		{415, "Unsupported Media Type"},
		{431, "Request Header Fields Too Large"},
		// Server Error
		{500, "Internal Server Error"},
		{501, "Not Implemented"},
		{502, "Bad Gateway"},
		{503, "Service Unavailable"},
		{504, "Gateway Timeout"},
		{505, "HTTP Version Not Supported"}
	};

	const std::string getReason(int code);

} // namespace HttpStatus
