#include "HttpResponse.hpp"
#include "../../Lib/Logger/Log.hpp"
#include <stdexcept>

HttpResponse::HttpResponse(const Config &conf)
	: _serverName(conf.getAppInfo().httpServerName), _statusCode(200),
	  _version(conf.getAppInfo().httpProtocolVersion), _isCgi(false),
	  _maxHeaderValueSize(conf.getMaxHeaderValueSize()),
	  _maxHeaderKeys(conf.getMaxHeaderKeys()),
	  _maxHeaderValuesPerKey(conf.getMaxHeaderValuesPerKey()),
	  _maxResponseBodySize(conf.getMaxResponseBodySize()) {}

HttpResponse::~HttpResponse() {}

bool HttpResponse::_isValidHeaderName(const std::string &name) {
	if (name.empty()) {
		return false;
	}

	for (size_t i = 0; i < name.size(); ++i) {
		const unsigned char c = static_cast< unsigned char >(name[i]);
		if (c <= 0x20 || c == ':' || c == 0x7F) {
			return false;
		}
	}
	return true;
}

bool HttpResponse::_isValidHeaderValue(const std::string &value) const {
	if (value.size() > _maxHeaderValueSize) {
		return false;
	}

	for (size_t i = 0; i < value.size(); ++i) {
		const unsigned char c = static_cast< unsigned char >(value[i]);
		if (c < 0x20 && c != 0x09) {
			return false;
		}
		if (c == 0x7F) {
			return false;
		}
	}
	return true;
}

bool HttpResponse::_isValidHttpVersion(const std::string &version) {
	return version == "HTTP/1.0" || version == "HTTP/1.1";
}

bool HttpResponse::_containsControlChars(const std::string &str) {
	for (size_t i = 0; i < str.size(); ++i) {
		const unsigned char c = static_cast< unsigned char >(str[i]);
		if (c < 0x20 || c == 0x7F) {
			return true;
		}
	}
	return false;
}

void HttpResponse::clear(const Config &c) {
	setStatusCode(200);
	setVersion(c.getAppInfo().httpProtocolVersion);
	_headers.clear();
	setBody("");
	_isCgi = false;
}

HttpResponse::HttpResponse(const HttpResponse &other)
	: _serverName(other._serverName), _statusCode(200),
	  _version(other._version), _isCgi(other._isCgi),
	  _maxHeaderValueSize(other._maxHeaderValueSize),
	  _maxHeaderKeys(other._maxHeaderKeys),
	  _maxHeaderValuesPerKey(other._maxHeaderValuesPerKey),
	  _maxResponseBodySize(other._maxResponseBodySize) {
	setStatusCode(other._statusCode);
	setBody(other._body);

	const std::map< std::string, std::vector< std::string > > &headers =
		other._headers;
	for (std::map< std::string, std::vector< std::string > >::const_iterator
			 it = headers.begin();
		 it != headers.end(); ++it) {
		for (std::vector< std::string >::const_iterator valIt =
				 it->second.begin();
			 valIt != it->second.end(); ++valIt) {
			appendHeader(it->first, *valIt);
		}
	}
}

HttpResponse &HttpResponse::operator=(const HttpResponse &other) {
	if (this != &other) {
		_serverName = other._serverName;
		setStatusCode(other._statusCode);
		setVersion(other._version);
		setBody(other._body);
		_isCgi = other._isCgi;
		_maxHeaderValueSize = other._maxHeaderValueSize;
		_maxHeaderKeys = other._maxHeaderKeys;
		_maxHeaderValuesPerKey = other._maxHeaderValuesPerKey;
		_maxResponseBodySize = other._maxResponseBodySize;

		_headers.clear();
		const std::map< std::string, std::vector< std::string > > &headers =
			other._headers;
		for (std::map< std::string, std::vector< std::string > >::const_iterator
				 it = headers.begin();
			 it != headers.end(); ++it) {
			for (std::vector< std::string >::const_iterator valIt =
					 it->second.begin();
				 valIt != it->second.end(); ++valIt) {
				appendHeader(it->first, *valIt);
			}
		}
	}
	return *this;
}

const std::string &HttpResponse::getServerName() const { return _serverName; }

void HttpResponse::setServerName(const std::string &name) {
	_serverName = name;
}

int HttpResponse::getStatusCode() const { return _statusCode; }

void HttpResponse::setStatusCode(const int code) {
	if (code < 100 || code > 599) {
		LOG(WARNING) << "Invalid status code attempted: " << code
					 << ". Using 500 instead.";
		_statusCode = 500;
		return;
	}
	_statusCode = code;
}

const std::string &HttpResponse::getVersion() const { return _version; }

void HttpResponse::setVersion(const std::string &version) {
	if (!_isValidHttpVersion(version)) {
		LOG(WARNING) << "Invalid HTTP version attempted: " << version
					 << ". Using " << AppInfo().httpProtocolVersion
					 << " instead.";
		_version = AppInfo().httpProtocolVersion;
		return;
	}
	_version = version;
}

const std::map< std::string, std::vector< std::string > > &
HttpResponse::getHeaders() const {
	return _headers;
}

const std::string &HttpResponse::getHeader(const std::string &key) const {
	const std::map< std::string, std::vector< std::string > >::const_iterator
		it = _headers.find(key);
	if (it != _headers.end() && !it->second.empty()) {
		return it->second[it->second.size() - 1];
	}
	static const std::string empty;
	return empty;
}

const std::vector< std::string > &
HttpResponse::getHeaderVector(const std::string &key) const {
	const std::map< std::string, std::vector< std::string > >::const_iterator
		it = _headers.find(key);
	if (it != _headers.end()) {
		return it->second;
	}
	static const std::vector< std::string > empty;
	return empty;
}

bool HttpResponse::hasHeader(const std::string &key) const {
	return _headers.count(key) > 0;
}

void HttpResponse::setHeader(const std::string &key, const std::string &value) {
	if (!_isValidHeaderName(key)) {
		LOG(WARNING) << "Invalid header name: " << key;
		return;
	}

	if (!_isValidHeaderValue(value)) {
		LOG(WARNING) << "Invalid header value for key: " << key;
		return;
	}

	if (_headers.size() >= _maxHeaderKeys &&
		_headers.find(key) == _headers.end()) {
		LOG(WARNING) << "Too many header keys. Maximum " << _maxHeaderKeys
					 << " allowed.";
		return;
	}

	_headers[key].clear();
	_headers[key].push_back(value);
}

void HttpResponse::appendHeader(const std::string &key,
								const std::string &value) {
	if (!_isValidHeaderName(key)) {
		LOG(WARNING) << "Invalid header name: " << key;
		return;
	}

	if (!_isValidHeaderValue(value)) {
		LOG(WARNING) << "Invalid header value for key: " << key;
		return;
	}

	if (_headers.size() >= _maxHeaderKeys &&
		_headers.find(key) == _headers.end()) {
		LOG(WARNING) << "Too many header keys. Maximum " << _maxHeaderKeys
					 << " allowed.";
		return;
	}

	if (_headers[key].size() >= _maxHeaderValuesPerKey) {
		LOG(WARNING) << "Too many values for header: " << key;
		return;
	}

	_headers[key].push_back(value);
}

const std::string &HttpResponse::getBody() const { return _body; }

void HttpResponse::setBody(const std::string &body) {
	if (body.size() > _maxResponseBodySize) {
		LOG(ERROR) << "Response body size exceeds maximum: " << body.size()
				   << " bytes (max: " << _maxResponseBodySize << ")";
		_statusCode = 500;
		_body = "";
		return;
	}
	_body = body;
}

bool HttpResponse::isCgi() const { return _isCgi; }
void HttpResponse::setIsCgi(bool isCgi) { _isCgi = isCgi; }
