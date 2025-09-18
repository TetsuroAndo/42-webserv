#include "HttpRequest.hpp"
#include "../../Lib/StringOps/StringOps.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

// Method
const std::string& HttpRequest::getMethod() const { return _method; }
void HttpRequest::setMethod(const std::string& method) { _method = method; }

// Path
const std::string& HttpRequest::getPath() const { return _path; }
void HttpRequest::setPath(const std::string& path) { _path = path; }

// Version
const std::string& HttpRequest::getVersion() const { return _version; }
void HttpRequest::setVersion(const std::string& version) { _version = version; }

// Headers
const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
	return _headers;
}
const std::string& HttpRequest::getHeader(const std::string& key) const {
	std::string lowerKey = key;
	StringOps::toLower(lowerKey);
	std::map<std::string, std::string>::const_iterator it = _headers.find(lowerKey);
	if (it != _headers.end()) {
		return it->second;
	}
	static const std::string empty;
	return empty;
}
bool HttpRequest::hasHeader(const std::string& key) const {
	std::string lowerKey = key;
	StringOps::toLower(lowerKey);
	return _headers.count(lowerKey) > 0;
}
void HttpRequest::addHeader(const std::string& key, const std::string& value) {
	std::string lowerKey = key;
	StringOps::toLower(lowerKey);
	_headers[lowerKey] = value;
}

// Queries
const std::map<std::string, std::string>& HttpRequest::getQueries() const {
	return _query;
}
const std::string& HttpRequest::getQuery(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = _query.find(key);
	if (it != _query.end()) {
		return it->second;
	}
	static const std::string empty;
	return empty;
}
bool HttpRequest::hasQuery(const std::string& key) const {
	return _query.count(key) > 0;
}
void HttpRequest::addQuery(const std::string& key, const std::string& value) {
	_query[key] = value;
}

// Body
const std::string& HttpRequest::getBody() const { return _body; }
void HttpRequest::setBody(const std::string& body) { _body = body; }
void HttpRequest::appendBody(const std::string& data) { _body.append(data); }

void HttpRequest::clear() {
	_method.clear();
	_path.clear();
	_version.clear();
	_headers.clear();
	_query.clear();
	_body.clear();
}
