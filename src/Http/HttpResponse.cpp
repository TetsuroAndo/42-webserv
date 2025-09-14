#include <ctime>
#include <sstream>

#include "HttpResponse.hpp"

HttpResponse::HttpResponse() : _statusCode(100), _body(""), _response("") {
	setHeaders("Server", "webserv");
	setHeaders("Connection", "close");
	setHeaders("Content-Length", "0");
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatusCode(int code) { _statusCode = code; }

std::string HttpResponse::getStatusLine(void) const {
	std::ostringstream oss;
	oss << "HTTP/1.0 " << _statusCode << " " << getStatusText();
	std::string str(oss.str());
	return str;
}

std::string HttpResponse::getStatusText(void) const {
	switch (_statusCode) {
	case 100:
		return "Continue";
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 204:
		return "No Content";
	case 301:
		return "Moved Permanently";
	case 302:
		return "Found";
	case 303:
		return "See Other";
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 408:
		return "Request Timeout";
	case 413:
		return "Content Too Large";
	case 414:
		return "URI Too Long";
	case 415:
		return "Unsupported Media Type";
	case 431:
		return "Request Header Fields Too Large";
	case 501:
		return "Not Implemented";
	case 502:
		return "Bad Gateway";
	case 503:
		return "Service Unavailable";
	case 504:
		return "Gateway Timeout";
	case 505:
		return "HTTP Version Not Supported";
	default:
		return "Internal Server Error";
	}
}

void HttpResponse::setHeaders(const std::string &headerTitle,
							  const std::string &headerValue) {
	_headers[headerTitle] = headerValue;
}

void HttpResponse::setResponseBody(const std::string &bodyMessage) {
	_body = bodyMessage;
	std::ostringstream oss;
	oss << _body.length();
	setHeaders("Content-Length", oss.str());
}

void HttpResponse::setDateHeader() {
	std::time_t now = std::time(NULL);
	std::tm *gmt = std::gmtime(&now);

	char buf[100];
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);

	setHeaders("Date", buf);
}

void HttpResponse::makeResponse(void) {
	_response.clear();
	_response += getStatusLine() + "\r\n";
	setDateHeader();
	for (std::map<std::string, std::string>::const_iterator it =
			 _headers.begin();
		 it != _headers.end(); ++it) {
		_response += it->first + ": " + it->second + "\r\n";
	}
	_response += "\r\n";
	_response += _body;
}
const std::string &HttpResponse::getResponse(void) {
	if (_response == "")
		makeResponse();
	return _response;
}
