#include "RequestParser.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpRequest.hpp"
#include "../Core/HttpStatus.hpp"

RequestParser::RequestParser() : _state(STATE_REQUEST_LINE), _errorCode(0) {}

RequestParser::~RequestParser() {}

void RequestParser::reset() {
	_state = STATE_REQUEST_LINE;
	_errorCode = 0;
	_bodyParser.reset();
}

RequestParser::ParseState RequestParser::getState() const { return _state; }

void RequestParser::setState(ParseState state) { _state = state; }

int RequestParser::getErrorCode() const { return _errorCode; }

void RequestParser::setErrorCode(int code) { _errorCode = code; }

bool RequestParser::isComplete() const { return _state == STATE_COMPLETE; }

ParseResult RequestParser::parseHead(HttpRequest &req, std::string &buffer) {
	switch (_state) {
	case STATE_REQUEST_LINE: {
		// リクエストラインの抽出（\r\nまで）
		const size_t crlfPos = buffer.find("\r\n");
		if (crlfPos == std::string::npos) {
			return PARSE_INCOMPLETE; // データが不完全
		}

		if (crlfPos == 0) {
			_errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}

		// リクエストラインをパース
		std::string line(buffer.begin(), buffer.begin() + crlfPos);
		buffer.erase(0, crlfPos + 2);

		int errorCode = 0;
		ParseResult result = _lineParser.parse(req, line, errorCode);
		if (result == PARSE_ERROR) {
			_errorCode = errorCode;
			return PARSE_ERROR;
		}

		_state = STATE_HEADERS;
	} // fallthrough
	case STATE_HEADERS: {
		// ヘッダーブロックの抽出（\r\n\r\nまで）
		const size_t headerEndPos = buffer.find("\r\n\r\n");
		if (headerEndPos == std::string::npos) {
			return PARSE_INCOMPLETE; // データが不完全
		}

		// ヘッダーブロックをパース
		std::string headerBlock(buffer.begin(), buffer.begin() + headerEndPos);
		buffer.erase(0, headerEndPos + 4);

		int errorCode = 0;
		ParseResult result = getHeadParser().parse(req, headerBlock, errorCode);
		if (result == PARSE_ERROR) {
			_errorCode = errorCode;
			return PARSE_ERROR;
		}

		// ヘッダー解析完了後、Content-LengthがmaxBodySizeを超えていないかチェック
		if (req.hasHeader("Content-Length")) {
			const std::string &lenStr = req.getHeader("Content-Length");
			size_t contentLength = 0;
			if (!StringOps::decStrToSize(lenStr, contentLength)) {
				_errorCode = HttpStatus::BAD_REQUEST;
				return PARSE_ERROR;
			}
			if (contentLength > req.getMaxBodySize()) {
				_errorCode = HttpStatus::PAYLOAD_TOO_LARGE;
				return PARSE_ERROR;
			}
		}

		_state = STATE_BODY;
		return PARSE_HEADERS_COMPLETE;
	}
	case STATE_BODY:
	case STATE_COMPLETE: {
		// 既にヘッダーパーシングが完了している
		return PARSE_HEADERS_COMPLETE;
	}
	default:
		_errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}
}

ParseResult RequestParser::parseBody(HttpRequest &req, std::string &buffer) {
	if (_state != STATE_BODY) {
		// ボディパーシングの状態でない場合はエラー
		_errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}

	// ボディのパーシングを行う（RequestBodyParser::parse()が自動的に初期化を行う）
	int errorCode = 0;
	ParseResult result;
	const size_t consumed = _bodyParser.parse(req, buffer, errorCode, result);

	if (consumed > 0) {
		buffer.erase(0, consumed);
	}

	// ボディサイズチェック（パーサー内部で行うべき責務）
	if (req.getBody().length() > req.getMaxBodySize()) {
		_errorCode = HttpStatus::PAYLOAD_TOO_LARGE;
		return PARSE_ERROR;
	}

	switch (result) {
	case PARSE_INCOMPLETE: {
		// ボディのパーシングが完了していない場合は待機
		return PARSE_INCOMPLETE;
	}
	case PARSE_ERROR: {
		_errorCode = errorCode;
		return PARSE_ERROR;
	}
	case PARSE_HEADERS_COMPLETE: {
		// ボディパーシング中にヘッダー完了が返ることはない（エラーとして扱う）
		_errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}
	case PARSE_COMPLETE: {
		// ボディパーシング完了
		_state = STATE_COMPLETE;
		return PARSE_COMPLETE;
	}
	default:
		_errorCode = HttpStatus::BAD_REQUEST;
		return PARSE_ERROR;
	}
}

RequestLineParser &RequestParser::getLineParser() { return _lineParser; }
RequestHeadParser &RequestParser::getHeadParser() { return _headParser; }
RequestBodyParser &RequestParser::getBodyParser() { return _bodyParser; }
