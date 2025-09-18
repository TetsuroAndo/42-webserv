#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpRequest.hpp"
#include "../Core/HttpStatus.hpp"
#include "RequestBodyParser.hpp"
#include <sstream>

RequestBodyParser::RequestBodyParser() {
	reset();
}
RequestBodyParser::~RequestBodyParser() {}

void RequestBodyParser::reset() {
	_state = UNINITIALIZED;
	_contentLengthRemaining = 0;
	_chunkSize = 0;
}

void RequestBodyParser::init(const HttpRequest& request, int& errorCode) {
	if (request.hasHeader("Transfer-Encoding")) {
		const std::string& encoding = request.getHeader("Transfer-Encoding");
		if (encoding == "chunked") {
			if (request.hasHeader("Content-Length")) {
				errorCode = HttpStatus::BAD_REQUEST;
				return;
			}
			_state = CHUNKED_SIZE;
		} else {
			errorCode = HttpStatus::NOT_IMPLEMENTED;
		}
	} else if (request.hasHeader("Content-Length")) {
		const std::string& lenStr = request.getHeader("Content-Length");
		std::stringstream ss(lenStr);
		ss >> _contentLengthRemaining;
		if (ss.fail() || !ss.eof()) {
			errorCode = HttpStatus::BAD_REQUEST;
			return;
		}
		if (_contentLengthRemaining == 0) {
			_state = COMPLETE;
		} else {
			_state = IDENTITY;
		}
	} else {
		_state = COMPLETE; // No body
	}
}

ParseResult RequestBodyParser::parse(HttpRequest& request, std::string& buffer, int& errorCode) {
	if (_state == UNINITIALIZED) {
		init(request, errorCode);
		if (errorCode != 0) return PARSE_ERROR;
		if (_state == COMPLETE) return PARSE_COMPLETE;
	}
	if (_state == IDENTITY) {
		return parseIdentity(request, buffer);
	} else if (_state >= CHUNKED_SIZE && _state <= CHUNKED_CRLF) {
		return parseChunked(request, buffer, errorCode);
	}
	return PARSE_COMPLETE;
}

// Content-Lengthに基づくボディ解析
ParseResult RequestBodyParser::parseIdentity(HttpRequest& request, std::string& buffer, int& errorCode) {
	if (buffer.length() == 0) {
		return PARSE_INCOMPLETE;
	}
	size_t to_read = std::min(buffer.length(), _contentLengthRemaining);
	request.appendBody(buffer.substr(0, to_read));
	buffer.erase(0, to_read);
	_contentLengthRemaining -= to_read;

	if (_contentLengthRemaining == 0) {
		_state = COMPLETE;
		return PARSE_COMPLETE;
	}
	return PARSE_INCOMPLETE;
}

ParseResult RequestBodyParser::parseChunked(HttpRequest& request, std::string& buffer, int& errorCode) {
	while (true) {
		if (_state == CHUNKED_SIZE) {
		size_t crlf_pos = buffer.find("\r\n");
		if (crlf_pos == std::string::npos) return PARSE_INCOMPLETE;

		std::string size_line = buffer.substr(0, crlf_pos);
		size_t semi_pos = size_line.find(';');
		if (semi_pos != std::string::npos) {
			size_line = size_line.substr(0, semi_pos);
		}
		std::stringstream ss(size_line);
		ss >> std::hex >> _chunkSize;
		if (ss.fail() || !ss.eof()) {
			errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}
		buffer.erase(0, crlf_pos + 2);

		if (_chunkSize == 0) {
			_state = CHUNKED_CRLF;
		} else {
			_state = CHUNKED_DATA;
			}
		}

		if (_state == CHUNKED_DATA) {
			if (buffer.length() < _chunkSize + 2) return PARSE_INCOMPLETE; // データ + CRLF

			request.appendBody(buffer.substr(0, _chunkSize));
			// チャンクデータの後のCRLFをチェック
			if (buffer.substr(_chunkSize, 2) != "\r\n") {
				errorCode = HttpStatus::BAD_REQUEST;
				return PARSE_ERROR;
			}
			buffer.erase(0, _chunkSize + 2);
			_state = CHUNKED_SIZE; // 次のチャンクサイズを読む
		}

		if (_state == CHUNKED_CRLF) {
			// trailer-partは無視し、最後のCRLFを待つ
			if (buffer.find("\r\n") == 0) {
				buffer.erase(0, 2);
				_state = COMPLETE;
				return PARSE_COMPLETE;
			}
			// Trailerヘッダを真面目にパースしない場合は、単に終端のCRLFを待つ
			if (buffer.length() < 2) return PARSE_INCOMPLETE;
			if (buffer[0] == '\r' && buffer[1] == '\n') { // 空のtrailer
				buffer.erase(0, 2);
				_state = COMPLETE;
				return PARSE_COMPLETE;
			}
			// (簡易実装) trailerヘッダがあっても無視して次のCRLFを探す
			size_t crlf_pos = buffer.find("\r\n");
			if (crlf_pos != std::string::npos) {
				buffer.erase(0, crlf_pos + 2);
			continue; // loop to check for the final CRLF
			}
			return PARSE_INCOMPLETE;
		}
	}
	return PARSE_INCOMPLETE;
}
