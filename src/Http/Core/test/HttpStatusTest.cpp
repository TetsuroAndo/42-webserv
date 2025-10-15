#include "HttpStatusTest.hpp"
#include "../HttpStatus.hpp"
#include <iostream>
#include <sstream>

#define PASS "\033[32m[PASS]\033[0m"
#define FAIL "\033[31m[FAIL]\033[0m"

#define ASSERT(cond, message)                                                  \
	do {                                                                       \
		if (!(cond)) {                                                         \
			std::cout << FAIL << " " << message << std::endl;                  \
			return;                                                            \
		}                                                                      \
		std::cout << PASS << " " << message << std::endl;                      \
	} while (0)

static void testAllDefinedCodes() {
	struct StatusEntry {
		int code;
		const char *reason;
	};
	static const StatusEntry entries[] = {
		{HttpStatus::CONTINUE, "Continue"},
		{HttpStatus::OK, "OK"},
		{HttpStatus::CREATED, "Created"},
		{HttpStatus::ACCEPTED, "Accepted"},
		{HttpStatus::NO_CONTENT, "No Content"},
		{HttpStatus::MOVED_PERMANENTLY, "Moved Permanently"},
		{HttpStatus::FOUND, "Found"},
		{HttpStatus::SEE_OTHER, "See Other"},
		{HttpStatus::TEMPORARY_REDIRECT, "Temporary Redirect"},
		{HttpStatus::PERMANENT_REDIRECT, "Permanent Redirect"},
		{HttpStatus::BAD_REQUEST, "Bad Request"},
		{HttpStatus::UNAUTHORIZED, "Unauthorized"},
		{HttpStatus::FORBIDDEN, "Forbidden"},
		{HttpStatus::NOT_FOUND, "Not Found"},
		{HttpStatus::METHOD_NOT_ALLOWED, "Method Not Allowed"},
		{HttpStatus::REQUEST_TIMEOUT, "Request Timeout"},
		{HttpStatus::PAYLOAD_TOO_LARGE, "Payload Too Large"},
		{HttpStatus::URI_TOO_LONG, "URI Too Long"},
		{HttpStatus::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
		{HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE,
		 "Request Header Fields Too Large"},
		{HttpStatus::INTERNAL_SERVER_ERROR, "Internal Server Error"},
		{HttpStatus::NOT_IMPLEMENTED, "Not Implemented"},
		{HttpStatus::BAD_GATEWAY, "Bad Gateway"},
		{HttpStatus::SERVICE_UNAVAILABLE, "Service Unavailable"},
		{HttpStatus::GATEWAY_TIMEOUT, "Gateway Timeout"},
		{HttpStatus::VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"}};

	for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); ++i) {
		std::string reason = HttpStatus::getReason(entries[i].code);

		std::ostringstream oss;
		oss << "Status " << entries[i].code << " => " << reason;

		ASSERT(reason == entries[i].reason, oss.str());
	}
}

static void testUnknownCode() {
	ASSERT(HttpStatus::getReason(999) == "Internal Server Error",
		   "Unknown code returns Internal Server Error");
}

void runHttpStatusTests() {
	testAllDefinedCodes();
	testUnknownCode();
}
