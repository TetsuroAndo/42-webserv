#include "CgiResponseParser.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include <istream>
#include <sstream>

CgiResponseParser::CgiResponseParser()
	: _statusCode(200), _statusMessage("OK"), _headersParsed(false) {}

CgiResponseParser::~CgiResponseParser() {}

bool CgiResponseParser::headersFound() const { return _headersParsed; }

void CgiResponseParser::parse(const std::string &rawResponse) {
	_headersParsed = false;
	std::string::size_type headerEndPos = rawResponse.find("\r\n\r\n");
	size_t headerEndLen = 4;

	if (headerEndPos == std::string::npos) {
		headerEndPos = rawResponse.find("\n\n");
		headerEndLen = 2;
	}

	if (headerEndPos == std::string::npos) {
		_body = rawResponse;
		return;
	}

	_headersParsed = true;
	const std::string headerBlock = rawResponse.substr(0, headerEndPos);
	_body = rawResponse.substr(headerEndPos + headerEndLen);

	_parseHeaders(headerBlock);
}

void CgiResponseParser::setResponse(HttpResponse &httpResponse) {
	httpResponse.setStatusCode(_statusCode);
	httpResponse.setBody(_body);
	httpResponse.setHeader("Content-Length", StringOps::toString(_body.size()));

	for (std::map< std::string, std::string >::const_iterator it =
			 _headers.begin();
		 it != _headers.end(); ++it) {
		httpResponse.setHeader(it->first, it->second);
	}
}

void CgiResponseParser::_parseHeaders(const std::string &headerBlock) {
	std::istringstream iss(headerBlock);
	std::string line;

	while (std::getline(iss, line)) {
		if (line.empty() || line == "\r") {
			continue;
		}

		const std::string::size_type colonPos = line.find(":");
		if (colonPos == std::string::npos) {
			continue;
		}

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t\r") + 1);

		std::string lowerKey = key;
		StringOps::toLower(lowerKey);
		if (lowerKey == "status") {
			std::istringstream statusIss(value);
			statusIss >> _statusCode;

			if (_statusCode < 100 || _statusCode > 599) {
				_statusCode = 500;
			}

			std::string messagePart;
			if (std::getline(statusIss, messagePart)) {
				_statusMessage =
					messagePart.substr(messagePart.find_first_not_of(" \t"));
			}
		} else {
			_headers[key] = value;
		}
	}
}
