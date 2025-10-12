#include "DummyHttpReq.hpp"
#include <algorithm>

DummyHttpReq::DummyHttpReq(const std::string& method, const std::string& path,
						   const std::map<std::string, std::string>& headers,
						   const std::string& body,
						   const std::map<std::string, std::string>& queries)
	: _method(method), _path(path), _headers(headers), _body(body), _queries(queries)
{}

const std::string& DummyHttpReq::getMethod() const { return _method; }
const std::string& DummyHttpReq::getPath() const { return _path; }

const std::string& DummyHttpReq::getHeader(const std::string& key) const {
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		std::string lower_key = it->first;
		std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
		if (lower_key == key) {
			return it->second;
		}
	}
	static const std::string empty = "";
	return empty;
}

const std::string& DummyHttpReq::getBody() const { return _body; }
const std::map<std::string, std::string>& DummyHttpReq::getQueries() const { return _queries; }
