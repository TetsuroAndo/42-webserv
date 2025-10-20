#include "CgiResponseParser.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include <istream>
#include <sstream>

CgiResponseParser::CgiResponseParser()
	: _statusCode(200), _statusMessage("OK"), _headers(), _body() {}

CgiResponseParser::~CgiResponseParser() {}

void CgiResponseParser::parse(const std::string &rawResponse) {
	std::string::size_type headerEndPos = rawResponse.find("\r\n\r\n");

	if (headerEndPos == std::string::npos) {
		// ヘッダとボディの区切りが見つからない場合は、すべてボディとして扱う
		_body = rawResponse;
		return;
	}

	std::string headerBlock = rawResponse.substr(0, headerEndPos);
	_body = rawResponse.substr(headerEndPos + 4);

	_parseHeaders(headerBlock);
}

void CgiResponseParser::setResponse(HttpResponse &httpResponse) {
	httpResponse.statusCode = _statusCode;
	httpResponse.statusMessage = _statusMessage;
	httpResponse.body = _body;
	httpResponse.headers["Content-Length"] = StringOps::toString(_body.size());

	for (std::map< std::string, std::string >::const_iterator it =
			 _headers.begin();
		 it != _headers.end(); ++it) {
		httpResponse.headers[it->first] = it->second;
	}
}

void CgiResponseParser::_parseHeaders(const std::string &headerBlock) {
	std::istringstream iss(headerBlock);
	std::string line;

	while (std::getline(iss, line)) {
		if (line.empty() || line == "\r") {
			continue;
		}

		std::string::size_type colonPos = line.find(":");
		if (colonPos == std::string::npos) {
			continue; // 不正な形式のヘッダ
		}

		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);

		// 前後の空白をトリム
		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t\r") + 1);

		// "Status"ヘッダは特別扱い
		std::string lowerKey = key;
		StringOps::toLower(lowerKey);
		if (lowerKey == "status") {
			std::istringstream statusIss(value);
			statusIss >> _statusCode;
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
