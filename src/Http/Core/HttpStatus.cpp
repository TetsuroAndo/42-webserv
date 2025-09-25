#include "HttpStatus.hpp"
#include <string>

struct StatusEntry {
	int code;
	const char *reason;
};

static const StatusEntry statusEntries[] = {
	{ HttpStatus::CONTINUE, "Continue" },
	// Success
	{ HttpStatus::OK, "OK" },
	{ HttpStatus::CREATED, "Created" },
	{ HttpStatus::ACCEPTED, "Accepted" },
	{ HttpStatus::NO_CONTENT, "No Content" },
	// Redirection
	{ HttpStatus::MOVED_PERMANENTLY, "Moved Permanently" },
	{ HttpStatus::FOUND, "Found" },
	{ HttpStatus::SEE_OTHER, "See Other" },
	// Client Error
	{ HttpStatus::BAD_REQUEST, "Bad Request" },
	{ HttpStatus::UNAUTHORIZED, "Unauthorized" },
	{ HttpStatus::FORBIDDEN, "Forbidden" },
	{ HttpStatus::NOT_FOUND, "Not Found" },
	{ HttpStatus::METHOD_NOT_ALLOWED, "Method Not Allowed" },
	{ HttpStatus::REQUEST_TIMEOUT, "Request Timeout" },
	{ HttpStatus::PAYLOAD_TOO_LARGE, "Payload Too Large" },
	{ HttpStatus::URI_TOO_LONG, "URI Too Long" },
	{ HttpStatus::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type" },
	{ HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large" },
	// Server Error
	{ HttpStatus::INTERNAL_SERVER_ERROR, "Internal Server Error" },
	{ HttpStatus::NOT_IMPLEMENTED, "Not Implemented" },
	{ HttpStatus::BAD_GATEWAY, "Bad Gateway" },
	{ HttpStatus::SERVICE_UNAVAILABLE, "Service Unavailable" },
	{ HttpStatus::GATEWAY_TIMEOUT, "Gateway Timeout" },
	{ HttpStatus::VERSION_NOT_SUPPORTED, "HTTP Version Not Supported" }
};
static const std::string unknown = "Internal Server Error";

std::string HttpStatus::getReason(int code) {
	for (size_t i = 0; i < sizeof(statusEntries) / sizeof(StatusEntry); ++i) {
		if (statusEntries[i].code == code)
			return std::string(statusEntries[i].reason);
	}
	return unknown;
}
