#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include <cstddef>
#include <string>
#include "ParseResult.hpp"
#include "RequestBodyParser.hpp"
#include "RequestHeadParser.hpp"
#include "RequestLineParser.hpp"

class HttpRequest;

/**
 * @brief リクエストパーシングの状態とパーサーインスタンスを管理するクラス
 * @note パーシング関連の責務を集約し、PipelineContextから分離する
 */
class RequestParser {
public:
	enum ParseState {
		STATE_REQUEST_LINE,
		STATE_HEADERS,
		STATE_BODY,
		STATE_COMPLETE
	};

	RequestParser(size_t maxHeaderBytes);
	~RequestParser();

	/// @brief パーサーと状態をリセットする
	void reset();

	/// @brief 現在のパーシング状態を取得する
	ParseState getState() const;
	/// @brief エラーコードを取得する
	int getErrorCode() const;
	/// @brief エラーコードを設定する
	void setErrorCode(int code);
	/// @brief パーシングが完了しているかどうかを返す
	bool isComplete() const;

	/**
	 * @brief リクエストラインをパースする
	 * @param req パース結果を格納するHttpRequestオブジェクト
	 * @param buffer 受信バッファ（パース済み部分は削除される）
	 * @return パース結果（PARSE_INCOMPLETE, PARSE_COMPLETE, PARSE_ERROR）
	 * @note 例: 'GET /auth/login.html HTTP/1.1\r\n\r\n'
	 */
	ParseResult parseRequestLine(HttpRequest &req, std::string &buffer);

	/**
	 * @brief ヘッダーブロックをパースする
	 * @param req パース結果を格納するHttpRequestオブジェクト
	 * @param buffer 受信バッファ（パース済み部分は削除される）
	 * @return パース結果（PARSE_INCOMPLETE, PARSE_COMPLETE, PARSE_ERROR）
	 * @note ヘッダーの構文解析のみを行う
	 */
	ParseResult parseHeaders(HttpRequest &req, std::string &buffer);

	/**
	 * @brief リクエストボディをパースする
	 * @param req
	 * パース結果を格納するHttpRequestオブジェクト
	 * @param buffer 受信バッファ（パース済み部分は削除される）
	 * @return パース結果（PARSE_INCOMPLETE, PARSE_COMPLETE, PARSE_ERROR）
	 * @note ボディサイズなどの制約はミドルウェア側で行う
	 */
	ParseResult parseBody(HttpRequest &req, std::string &buffer);

	/// @brief リクエストラインパーサーへの参照を取得する
	RequestLineParser &getLineParser();
	/// @brief ヘッダーパーサーへの参照を取得する
	RequestHeadParser &getHeadParser();
	/// @brief ボディパーサーへの参照を取得する
	RequestBodyParser &getBodyParser();

private:
	int _errorCode;
	ParseState _state;
	size_t _maxHeaderBytes;

	RequestLineParser _lineParser;
	RequestHeadParser _headParser;
	RequestBodyParser _bodyParser;

	RequestParser(const RequestParser &);
	RequestParser &operator=(const RequestParser &);
};

#endif
