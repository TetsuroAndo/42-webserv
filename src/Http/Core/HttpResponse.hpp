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

	// Status Message
	const std::string &getStatusMessage() const;
	void setStatusMessage(const std::string &message);

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

	void clear();

	HttpResponse(const HttpResponse &other)
		: _serverName(other._serverName), _statusCode(other._statusCode),
		  _version(other._version), _headers(other._headers),
		  _body(other._body) {}

	HttpResponse &operator=(const HttpResponse &other) {
		if (this != &other) {
			_serverName = other._serverName;
			_statusCode = other._statusCode;
			_version = other._version;
			_headers = other._headers;
			_body = other._body;
		}
		return *this;
	}

private:
	std::string _serverName;
	int _statusCode;
	std::string _statusMessage;
	std::string _version;
	std::map< std::string, std::vector< std::string > > _headers;
	std::string _body;

	HttpResponse();
};

#endif
