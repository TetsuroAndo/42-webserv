#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>

class HttpResponse {
private:
	int _statusCode;
	std::map<std::string, std::string> _headers;
	std::string _body;
	std::string _response;

public:
	HttpResponse();
	~HttpResponse();

	void setStatusCode(int code);
	std::string getStatusText(void) const;
	std::string getStatusLine(void) const;
	void setHeaders(const std::string &headerTitle,
					const std::string &headerValue);
	void setResponseBody(const std::string &bodyMessage);
	void setDateHeader(void);
	const std::string &getResponse(void);
};

#endif
