#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../../Config/Config.hpp"
#include <map>
#include <string>
#include <vector>

class HttpResponse {
public:
	HttpResponse(const Config &conf);
	~HttpResponse();

	const std::string &getServerName() const;
	void setServerName(const std::string &name);

	int getStatusCode() const;
	void setStatusCode(int code);

	const std::string &getVersion() const;
	void setVersion(const std::string &version);

	const std::map< std::string, std::vector< std::string > > &
	getHeaders() const;
	const std::string &getHeader(const std::string &key) const;
	const std::vector< std::string > &
	getHeaderVector(const std::string &key) const;
	bool hasHeader(const std::string &key) const;
	void setHeader(const std::string &key, const std::string &value);
	void appendHeader(const std::string &key, const std::string &value);

	const std::string &getBody() const;
	void setBody(const std::string &body);

	bool isCgi() const;
	void setIsCgi(bool isCgi);

	void clear(const Config &c);

	HttpResponse(const HttpResponse &other);

	HttpResponse &operator=(const HttpResponse &other);

private:
	std::string _serverName;
	int _statusCode;
	std::string _statusMessage;
	std::string _version;
	std::map< std::string, std::vector< std::string > > _headers;
	std::string _body;
	bool _isCgi;

	size_t _maxHeaderValueSize;
	size_t _maxHeaderKeys;
	size_t _maxHeaderValuesPerKey;
	size_t _maxResponseBodySize;

	static bool _isValidHeaderName(const std::string &name);
	bool _isValidHeaderValue(const std::string &value) const;
	static bool _isValidHttpVersion(const std::string &version);
	static bool _containsControlChars(const std::string &str);
};

#endif
