#pragma once

#include <map>
#include <string>

#include "../../Config/Config.hpp"

class HttpRequest {
public:
	HttpRequest(const Config& config);
	~HttpRequest();

	// Max
	size_t getMaxBodySize() const;
	void setMaxBodySize(size_t size);

	// Method
	const std::string &getMethod() const;
	void setMethod(const std::string &method);

	// Path (URIの?より前の部分)
	const std::string &getPath() const;
	void setPath(const std::string &path);

	// HTTP Version
	const std::string &getVersion() const;
	void setVersion(const std::string &version);

	// Headers
	const std::map<std::string, std::vector<std::string> > &getHeaders() const;
	const std::string &getHeader(const std::string &key) const;
	const std::vector<std::string> &getHeaderVector(const std::string &key) const;
	bool hasHeader(const std::string &key) const;
	bool hasHeader(const char *keyStart, size_t keyLen) const;
	void addHeader(const std::string &key, const std::string &value);
	void addHeader(const char *keyStart, size_t keyLen, const char *valStart,
				   size_t valLen);
	void appendHeader(const std::string&key, const std::string&value);
	void appendHeader(const char*keyStart, size_t keyLen, const char*valStart, size_t valLen);
	// Query Parameters (?以降のキーバリュー)
	const std::map<std::string, std::string> &getQueries() const;
	const std::string &getQuery(const std::string &key) const;
	bool hasQuery(const std::string &key) const;
	void addQuery(const std::string &key, const std::string &value);

	// Body
	const std::string &getBody() const;
	void setBody(const std::string &body);
	void appendBody(const std::string &data);
	void appendBody(const char *data, size_t len);

	// 内部状態をリセット
	void clear();

private:
	size_t maxBodySize;

	std::string _method;
	std::string _path;
	std::string _version;
	std::map<std::string, std::vector<std::string> > _headers;
	std::map<std::string, std::string> _query;
	std::string _body;

	HttpRequest(const HttpRequest &);
	HttpRequest &operator=(const HttpRequest &);
};
