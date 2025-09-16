#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <string>

enum ParseErrorStatus {
	NONE,
	PARSE_ERROR_LARGE_REQUEST = 413,
	PARSE_ERROR_LARGE_HEADER = 431,
	PARSE_ERROR_INVALID_REQUEST = 500,
	PARSE_ERROR_HTTP_METHOD = 501,
	PARSE_ERROR_HTTP_VERSION = 505
};

enum ParseStatus { PARSE_COMPLETE, PARSE_INCOMPLETE, PARSE_ERROR };

class HttpRequest {
private:
	ParseStatus _parseStatus;
	ParseErrorStatus _error;
	const static size_t maxBodySize = 10 * 1024 * 1024;
	const static size_t maxHeaderSize = 8192;

	std::string _method;
	std::string _path;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::map<std::string, std::string> _query;
	std::string _body;

	std::string _queryString;

	bool parseRequestLine(std::string &requestLine);
	bool parseHeaders(std::istringstream &headerStream);
	bool parseBody(std::string &buffer, size_t bodyStart);

	bool splitRequestLine(const std::string &requestLine);
	void splitPathAndQuery();
	void parseQueryString();

	bool parseChunkedBody(std::string &buffer, size_t bodyStart);
	bool parseContentLengthBody(std::string &buffer, size_t bodyStart);

public:
	HttpRequest();
	~HttpRequest();

	ParseStatus parse(std::string &buffer);
	bool isComplete() const;
	void setError(ParseErrorStatus status);

	const std::string &getMethod() const;
	const std::string &getPath() const;
	const std::string &getVersion() const;
	const std::string &getBody() const;
	const std::string &getHeader(const std::string &header) const;
	int getError() const;

	void printData();
};

#endif
