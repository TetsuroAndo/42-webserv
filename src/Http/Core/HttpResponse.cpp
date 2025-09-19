#include "HttpResponse.hpp"
#include <stdexcept>

HttpResponse::HttpResponse(const std::string &serverName) : _statusCode(200), _version(HTTP_VERSION) {
	setServerName(serverName);
}

HttpResponse::~HttpResponse() {}

void HttpResponse::clear() {
	_statusCode = 200;
	_version = HTTP_VERSION;
	_headers.clear();
	_body.clear();
}

// Server Name
const std::string &HttpResponse::getServerName() const { 
	return getHeader("Server");
}

bool HttpResponse::setServerName(const std::string &name) {
	if (hasHeader("Server")) return false;
	setHeader("Server", name);
	return true;
}

// Status Code
int HttpResponse::getStatusCode() const { return _statusCode; }
void HttpResponse::setStatusCode(int code) { _statusCode = code; }

// Version
const std::string &HttpResponse::getVersion() const { return _version; }
void HttpResponse::setVersion(const std::string &version) { _version = version; }

// Headers
const std::map<std::string, std::string> &HttpResponse::getHeaders() const {
	return _headers;
}

const std::string &HttpResponse::getHeader(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end()) {
		return it->second;
	}
	static const std::string empty;
	return empty;
}

bool HttpResponse::hasHeader(const std::string &key) const {
	return _headers.count(key) > 0;
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
	_headers[key] = value;
}

// Body
const std::string &HttpResponse::getBody() const { return _body; }
void HttpResponse::setBody(const std::string &body) { _body = body; }
