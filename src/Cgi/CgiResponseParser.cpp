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
	const size_t bodyStartPos = headerEndPos + headerEndLen;
	if (bodyStartPos <= rawResponse.size()) {
		_body = rawResponse.substr(bodyStartPos);
	} else {
		_body.clear();
	}

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

		size_t keyStart = key.find_first_not_of(" \t");
		if (keyStart != std::string::npos) {
			key.erase(0, keyStart);
		} else {
			key.clear();
		}
		size_t keyEnd = key.find_last_not_of(" \t");
		if (keyEnd != std::string::npos) {
			key.erase(keyEnd + 1);
		} else {
			key.clear();
		}

		size_t valueStart = value.find_first_not_of(" \t");
		if (valueStart != std::string::npos) {
			value.erase(0, valueStart);
		} else {
			value.clear();
		}
		size_t valueEnd = value.find_last_not_of(" \t\r");
		if (valueEnd != std::string::npos) {
			value.erase(valueEnd + 1);
		} else {
			value.clear();
		}

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
				size_t msgStart = messagePart.find_first_not_of(" \t");
				if (msgStart != std::string::npos) {
					_statusMessage = messagePart.substr(msgStart);
				} else {
					_statusMessage = "";
				}
			}
		} else {
			_headers[key] = value;
		}
	}
}
