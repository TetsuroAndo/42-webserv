#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpRequest.hpp"
#include "../Core/HttpStatus.hpp"
#include "RequestBodyParser.hpp"
#include <sstream>
#include <ctime>

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
		return parseIdentity(request, buffer, errorCode);
	} else if (_state >= CHUNKED_SIZE && _state <= CHUNKED_CRLF) {
		return parseChunked(request, buffer, errorCode);
	}
	return PARSE_COMPLETE;
}

ParseResult RequestBodyParser::parseIdentity(HttpRequest& request, std::string& buffer, int& errorCode) {
	if (buffer.length() > _contentLengthRemaining) {
		errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}
	if (buffer.length() == 0) {
		return PARSE_INCOMPLETE;
	}
	size_t to_read = buffer.length();
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
	static time_t lastReceiveTime = time(NULL);
	const int TIMEOUT_SECONDS = 10; // TODO: ちゃんとしたタイムアウト時間を設定。暫定Timeout値

	while (true) {
		time_t now = time(NULL);
		if (now - lastReceiveTime > TIMEOUT_SECONDS) {
			errorCode = HttpStatus::REQUEST_TIMEOUT;
			return PARSE_ERROR;
		}
		if (!buffer.empty()) {
			lastReceiveTime = now;
		}

		if (_state == CHUNKED_SIZE) {
			size_t crlf_pos = buffer.find("\r\n");
			if (crlf_pos == std::string::npos) return PARSE_INCOMPLETE;

			std::string size_line = buffer.substr(0, crlf_pos);

			// チャンク拡張(chunk-extension)があれば無視する (例: "a;foo=bar")
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
			if (buffer.length() < _chunkSize + 2) return PARSE_INCOMPLETE;

			// チャンクデータをリクエストボディに追加
			request.appendBody(buffer.substr(0, _chunkSize));

			// チャンクデータの直後がCRLFであることを確認
			if (buffer.substr(_chunkSize, 2) != "\r\n") {
				errorCode = HttpStatus::BAD_REQUEST;
				return PARSE_ERROR;
			}
			
			// 処理した部分（データ + CRLF）をバッファから削除
			buffer.erase(0, _chunkSize + 2);
			_state = CHUNKED_SIZE; // 次のチャンクサイズを読む
		}

		if (_state == CHUNKED_CRLF) {
			while (true) {
				size_t crlf_pos = buffer.find("\r\n");
				if (crlf_pos == std::string::npos) {
					return PARSE_INCOMPLETE;
				}

				// crlf_posが0なら、バッファの先頭がCRLF、つまり空行
				if (crlf_pos == 0) {
					buffer.erase(0, 2); // 最後のCRLFを消費
					_state = COMPLETE;
					return PARSE_COMPLETE;
				}

				// 空行でなければトレーラーヘッダなので、その行を読み飛ばす
				buffer.erase(0, crlf_pos + 2);
				// bufferに次の行が残っていれば、このwhileループで引き続き評価する
			}
		}
		if (_state == COMPLETE) {
			return PARSE_COMPLETE;
		}
	}
	return PARSE_INCOMPLETE;
}
