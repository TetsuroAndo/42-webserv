#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>

#define HTTP_VERSION "HTTP/1.0"
#define SERVER_NAME "webserv/42"

class HttpResponse {
public:
	HttpResponse(const std::string &serverName = SERVER_NAME);
	~HttpResponse();

	// Server Name
	const std::string &getServerName() const;
	void setServerName(const std::string &name = SERVER_NAME);

	// Status Code
	int getStatusCode() const;
	void setStatusCode(int code);

	// HTTP Version
	const std::string &getVersion() const;
	void setVersion(const std::string &version);

	// Headers
	const std::map<std::string, std::string> &getHeaders() const;
	const std::string &getHeader(const std::string &key) const;
	bool hasHeader(const std::string &key) const;
	void setHeader(const std::string &key, const std::string &value);

	// Body
	const std::string &getBody() const;
	void setBody(const std::string &body);

	void clear();

	HttpResponse(const HttpResponse &other)
		: _serverName(other._serverName), _statusCode(other._statusCode),
		  _version(other._version), _headers(other._headers), _body(other._body) {}

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
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _body;

	HttpResponse();
};

#endif
