#include "HttpStatus.hpp"
#include <string>
#include <map>

namespace {
	typedef std::map<int, std::string> StatusMap;
	StatusMap createStatusReasons() {
		StatusMap code;
		// Informational
		code[100] = "Continue";
		// Success
		code[200] = "OK";
		code[201] = "Created";
		code[202] = "Accepted";
		code[204] = "No Content";
		// Redirection
		code[301] = "Moved Permanently";
		code[302] = "Found";
		code[303] = "See Other";
		// Client Error
		code[400] = "Bad Request";
		code[401] = "Unauthorized";
		code[403] = "Forbidden";
		code[404] = "Not Found";
		code[405] = "Method Not Allowed";
		code[408] = "Request Timeout";
		code[413] = "Payload Too Large";
		code[414] = "URI Too Long";
		code[415] = "Unsupported Media Type";
		code[431] = "Request Header Fields Too Large";
		/// Server Error
		code[500] = "Internal Server Error";
		code[501] = "Not Implemented";
		code[502] = "Bad Gateway";
		code[503] = "Service Unavailable";
		code[504] = "Gateway Timeout";
		code[505] = "HTTP Version Not Supported";
		return code;
	}

	// Static
	const StatusMap statusReasons = createStatusReasons();
	const std::string unknown = "Internal Server Error";

} // namespace

const std::string &HttpStatus::getReason(int code) {
	StatusMap::const_iterator it = statusReasons.find(code);
	if (it != statusReasons.end())
		return it->second;
	return unknown;
}
