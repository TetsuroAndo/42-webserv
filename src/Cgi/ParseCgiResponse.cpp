#include "ParseCgiResponse.hpp"
#include <sstream>
#include <iostream>

CgiResponseParser::CgiResponseParser() : _statusCode(200), _statusMessage("OK") {}

CgiResponseParser::~CgiResponseParser() {}

void CgiResponseParser::parse(const std::string& rawResponse) {
	std::string::size_type header_end_pos = rawResponse.find("\r\n\r\n");

	if (header_end_pos == std::string::npos) {
		// ヘッダとボディの区切りが見つからない場合、すべてがボディである可能性がある
		// non-parsed-header CGI
		_cgiBodyStr = rawResponse;
	} else {
		_cgiHeadersStr = rawResponse.substr(0, header_end_pos);
		_cgiBodyStr = rawResponse.substr(header_end_pos + 4);
		_parseHeaders();
	}
}

void CgiResponseParser::setResponse(HttpResponse& httpResponse) {
	httpResponse.setStatusCode(_statusCode);
	httpResponse.setStatusMessage(_statusMessage);

	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it) {
		httpResponse.setHeader(it->first, it->second);
	}
	httpResponse.setBody(_cgiBodyStr);

	// Bodyの長さに応じてContent-Lengthをセットする
	if (_headers.find("Content-Length") == _headers.end()) {
		std::stringstream ss;
		ss << _cgiBodyStr.length();
		httpResponse.setHeader("Content-Length", ss.str());
	}
}

void CgiResponseParser::_parseHeaders() {
	std::stringstream ss(_cgiHeadersStr);
	std::string line;

	while (std::getline(ss, line)) {
		if (line.empty() || line == "\r") {
			continue;
		}
		// 行末の\rを削除
		if (!line.empty() && line[line.size() - 1] == '\r') {
			line.erase(line.size() - 1);
		}

		std::string::size_type colon_pos = line.find(':');
		if (colon_pos == std::string::npos) {
			continue; // 不正なヘッダ行
		}

		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);

		// 先頭の空白を削除
		std::string::size_type first_char = value.find_first_not_of(" \t");
		if (first_char != std::string::npos) {
			value = value.substr(first_char);
		}

		// 特別なヘッダの処理
		if (key == "Status") {
			std::stringstream status_ss(value);
			status_ss >> _statusCode;
			std::getline(status_ss, _statusMessage);
			// メッセージの先頭の空白を削除
			first_char = _statusMessage.find_first_not_of(" \t");
			if (first_char != std::string::npos) {
				_statusMessage = _statusMessage.substr(first_char);
			}
		} else {
			_headers[key] = value;
		}
	}
}
