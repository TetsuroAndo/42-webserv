#include "RequestParser.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpRequest.hpp"
#include "../Core/HttpStatus.hpp"

RequestParser::RequestParser(const Config &config)
	: _errorCode(0), _state(STATE_REQUEST_LINE), _bodyParser(config),
	  _config(config) {}

RequestParser::~RequestParser() {}

// ヘッダーの最大許容サイズ (e.g., 8KB)
// 悪意のあるクライアントが改行を送らずにデータを送り続ける攻撃を防ぐ

void RequestParser::reset() {
	_state = STATE_REQUEST_LINE;
	_errorCode = 0;
	_bodyParser.reset();
}

RequestParser::ParseState RequestParser::getState() const { return _state; }
int RequestParser::getErrorCode() const { return _errorCode; }
void RequestParser::setErrorCode(const int code) { _errorCode = code; }
bool RequestParser::isComplete() const { return _state == STATE_COMPLETE; }

ParseResult RequestParser::parseRequestLine(HttpRequest &req,
											std::string &buffer) {
	if (_state != STATE_REQUEST_LINE) {
		LOG(WARNING) << "parseRequestLine called in invalid state: " << _state;
		_errorCode = HttpStatus::INTERNAL_SERVER_ERROR;
		return PARSE_ERROR;
	}

	// DoS対策: バッファが最大ヘッダーサイズを超えたらエラー
	if (_config.getMaxRequestHeaderSize() < buffer.size()) {
		_errorCode = HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE;
		return PARSE_ERROR;
	}

	const size_t crlfPos = buffer.find("\r\n");
	if (crlfPos == std::string::npos) {
		return PARSE_INCOMPLETE; // まだ改行が来ていない
	}
	if (crlfPos == 0) {
		_errorCode = HttpStatus::BAD_REQUEST; // 空行
		return PARSE_ERROR;
	}

	const std::string line(buffer.begin(), buffer.begin() + crlfPos);
	if (!_lineParser.parse(req, line, _errorCode)) {
		return PARSE_ERROR; // lineParserがfalseを返したら_errorCodeが設定されている
	}

	LOG(DEBUG) << "RequestParser::parseRequestLine: Successfully parsed"
			   << attr("method", req.getMethod()) << attr("path", req.getPath())
			   << attr("version", req.getVersion())
			   << attr("path_empty", req.getPath().empty());

	buffer.erase(0, crlfPos + 2); // パースした分をバッファから削除
	_state = STATE_HEADERS;		  // 次の状態に遷移
	return PARSE_COMPLETE;		  // このステップの完了
}

ParseResult RequestParser::parseHeaders(HttpRequest &req, std::string &buffer) {
	if (_state != STATE_HEADERS) {
		LOG(WARNING) << "parseHeaders called in invalid state: " << _state;
		_errorCode = HttpStatus::INTERNAL_SERVER_ERROR;
		return PARSE_ERROR;
	}

	const size_t headerEndPos = buffer.find("\r\n\r\n");
	if (headerEndPos == std::string::npos) {
		// ヘッダーが終わっていないが、サイズ制限は超えていないか再度チェック
		if (_config.getMaxRequestHeaderSize() < buffer.size()) {
			_errorCode = HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE;
			return PARSE_ERROR;
		}
		return PARSE_INCOMPLETE;
	}

	const std::string headerBlock(buffer.begin(),
								  buffer.begin() + headerEndPos);
	if (!_headParser.parse(req, headerBlock, _errorCode)) {
		return PARSE_ERROR;
	}

	buffer.erase(0, headerEndPos + 4); // パースした分をバッファから削除

	// Content-Lengthのチェック
	const bool hasContentLength = req.hasHeader("Content-Length");
	size_t contentLength = 0;
	if (hasContentLength) {
		const std::string &lenStr = req.getHeader("Content-Length");
		if (!StringOps::decStrToSize(lenStr, contentLength)) {
			_errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}
		if (req.getMaxBodySize() < contentLength) {
			_errorCode = HttpStatus::PAYLOAD_TOO_LARGE;
			return PARSE_ERROR;
		}
	}

	const bool hasTransferEncoding = req.hasHeader("Transfer-Encoding");

	if (!hasTransferEncoding && (!hasContentLength || contentLength == 0)) {
		if (hasContentLength && contentLength == 0 && !buffer.empty()) {
			_errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}
		_state = STATE_COMPLETE;
	} else {
		_state = STATE_BODY;
	}
	return PARSE_COMPLETE;
}

ParseResult RequestParser::parseBody(HttpRequest &req, std::string &buffer) {
	if (_state != STATE_BODY) {
		LOG(WARNING) << "parseBody called in invalid state: " << _state;
		_errorCode = HttpStatus::INTERNAL_SERVER_ERROR;
		return PARSE_ERROR;
	}

	// ボディのパーシングを行う
	ParseResult result;
	const size_t consumed = _bodyParser.parse(req, buffer, _errorCode, result);

	// エラーが発生した場合は即座に返す
	if (result == PARSE_ERROR) {
		return PARSE_ERROR;
	}

	if (0 < consumed) {
		if (result == PARSE_COMPLETE && req.hasHeader("Content-Length") &&
			!req.hasHeader("Transfer-Encoding")) {
			if (buffer.length() > consumed) {
				_errorCode = HttpStatus::BAD_REQUEST;
				return PARSE_ERROR;
			}
		}
		buffer.erase(0, consumed);
	}

	// ボディサイズチェック
	if (req.getMaxBodySize() < req.getBody().length()) {
		_errorCode = HttpStatus::PAYLOAD_TOO_LARGE;
		return PARSE_ERROR;
	}

	if (result == PARSE_COMPLETE) {
		_state = STATE_COMPLETE;
	}

	return result;
}

RequestLineParser &RequestParser::getLineParser() { return _lineParser; }
RequestHeadParser &RequestParser::getHeadParser() { return _headParser; }
RequestBodyParser &RequestParser::getBodyParser() { return _bodyParser; }
