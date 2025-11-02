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

	// Server Name
	const std::string &getServerName() const;
	void setServerName(const std::string &name);

	// Status Code
	int getStatusCode() const;
	void setStatusCode(int code);

	// HTTP Version
	const std::string &getVersion() const;
	void setVersion(const std::string &version);

	// Headers
	const std::map< std::string, std::vector< std::string > > &
	getHeaders() const;
	const std::string &getHeader(const std::string &key) const;
	const std::vector< std::string > &
	getHeaderVector(const std::string &key) const;
	bool hasHeader(const std::string &key) const;
	void setHeader(const std::string &key, const std::string &value);
	void appendHeader(const std::string &key, const std::string &value);

	// Body
	const std::string &getBody() const;
	void setBody(const std::string &body);

	// directoryFlag
	bool isDirectoryResponse() const;
	void setIsDirectoryResponse(bool isDirectory);

	void clear(const Config &c);

	HttpResponse(const HttpResponse &other)
		: _serverName(other._serverName), _statusCode(other._statusCode),
		  _version(other._version), _headers(other._headers),
		  _body(other._body), _isDirectory(other._isDirectory) {}

	HttpResponse &operator=(const HttpResponse &other) {
		if (this != &other) {
			_serverName = other._serverName;
			_statusCode = other._statusCode;
			_version = other._version;
			_headers = other._headers;
			_body = other._body;
			_isDirectory = other._isDirectory;
		}
		return *this;
	}

private:
	std::string _serverName;
	int _statusCode;
	std::string _version;
	std::map< std::string, std::vector< std::string > > _headers;
	std::string _body;
	bool _isDirectory;

	HttpResponse()
		: _serverName(""), _statusCode(200), _version("HTTP/1.1"), _headers(),
		  _body(""), _isDirectory(false) {}
};

#endif
