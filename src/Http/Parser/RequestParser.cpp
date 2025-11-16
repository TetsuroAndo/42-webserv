#include "RequestParser.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../Core/HttpRequest.hpp"
#include "../Core/HttpStatus.hpp"

RequestParser::RequestParser()
	: _errorCode(HttpStatus::OK), _state(STATE_REQUEST_LINE) {}

RequestParser::~RequestParser() {}

// ヘッダーの最大許容サイズ (e.g., 8KB)
// 悪意のあるクライアントが改行を送らずにデータを送り続ける攻撃を防ぐ
static const size_t MAX_REQ_HEADER_SIZE = 8192;

void RequestParser::reset() {
	_state = STATE_REQUEST_LINE;
	_errorCode = HttpStatus::OK;
	_bodyParser.reset();
}

RequestParser::ParseState RequestParser::getState() const { return _state; }
int RequestParser::getErrorCode() const { return _errorCode; }
void RequestParser::setErrorCode(int code) { _errorCode = code; }
bool RequestParser::isComplete() const { return _state == STATE_COMPLETE; }

ParseResult RequestParser::parseRequestLine(HttpRequest &req,
											std::string &buffer) {
	if (_state != STATE_REQUEST_LINE) {
		LOG(WARNING) << "parseRequestLine called in invalid state: " << _state;
		_errorCode = HttpStatus::INTERNAL_SERVER_ERROR;
		return PARSE_ERROR;
	}

	// DoS対策: バッファが最大ヘッダーサイズを超えたらエラー
	if (buffer.size() > MAX_REQ_HEADER_SIZE) {
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

	std::string line(buffer.begin(), buffer.begin() + crlfPos);
	if (!_lineParser.parse(req, line, _errorCode)) {
		return PARSE_ERROR; // lineParserがfalseを返したら_errorCodeが設定されている
	}

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
		if (buffer.size() > MAX_REQ_HEADER_SIZE) {
			_errorCode = HttpStatus::REQUEST_HEADER_FIELDS_TOO_LARGE;
			return PARSE_ERROR;
		}
		return PARSE_INCOMPLETE;
	}

	std::string headerBlock(buffer.begin(), buffer.begin() + headerEndPos);
	if (!_headParser.parse(req, headerBlock, _errorCode)) {
		return PARSE_ERROR;
	}

	buffer.erase(0, headerEndPos + 4); // パースした分をバッファから削除

	// Content-Lengthのチェック
	if (req.hasHeader("Content-Length")) {
		const std::string &lenStr = req.getHeader("Content-Length");
		size_t contentLength = 0;
		if (!StringOps::decStrToSize(lenStr, contentLength)) {
			_errorCode = HttpStatus::BAD_REQUEST;
			return PARSE_ERROR;
		}
		// この時点で設定されている maxBodySize (Location固有値) と比較
		if (contentLength > req.getMaxBodySize()) {
			_errorCode = HttpStatus::PAYLOAD_TOO_LARGE;
			return PARSE_ERROR;
		}
	}

	_state = STATE_BODY;
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

	if (consumed > 0) {
		buffer.erase(0, consumed);
	}

	// ボディサイズチェック（パーサー内部で行うべき責務）
	if (req.getBody().length() > req.getMaxBodySize()) {
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
