#include "HttpRequest.hpp"
#include "../../Lib/StringOps/StringOps.hpp"

HttpRequest::HttpRequest() {}
HttpRequest::~HttpRequest() {}

// Max
size_t HttpRequest::getMaxHeaderSize() const { return maxHeaderSize; }
void HttpRequest::setMaxHeaderSize(const size_t size) { maxHeaderSize = size; }
size_t HttpRequest::getMaxBodySize() const { return maxBodySize; }
void HttpRequest::setMaxBodySize(const size_t size) { maxBodySize = size; }

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
	StringOps::toLower(const_cast<std::string &>(key));
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end()) {
		return it->second;
	}
	static const std::string empty;
	return empty;
}
bool HttpRequest::hasHeader(const std::string& key) const {
	StringOps::toLower(const_cast<std::string &>(key));
	return _headers.count(key) > 0;
}

bool HttpRequest::hasHeader(const char* key_start, size_t key_len) const {
	std::string key(key_start, key_len);
	StringOps::toLower(key);
	return _headers.count(key) > 0;
}

void HttpRequest::addHeader(const std::string& key, const std::string& value) {
	StringOps::toLower(const_cast<std::string&>(key));
	_headers[key] = value;
}

void HttpRequest::addHeader(const char* key_start, size_t key_len, const char* val_start, size_t val_len) {
	std::string key(key_start, key_len);
	StringOps::toLower(key);
	_headers[key] = std::string(val_start, val_len);
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
void HttpRequest::appendBody(const char* data, size_t len) { _body.append(data, len); }

void HttpRequest::clear() {
	_method.clear();
	_path.clear();
	_version.clear();
	_headers.clear();
	_query.clear();
	_body.clear();
}
