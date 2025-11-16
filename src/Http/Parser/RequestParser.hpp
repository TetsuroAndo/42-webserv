#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

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

	RequestParser();
	~RequestParser();

	/// @brief パーサーと状態をリセットする
	void reset();

	/// @brief 現在のパーシング状態を取得する
	ParseState getState() const;
	/// @brief パーシング状態を設定する
	void setState(ParseState state);
	/// @brief エラーコードを取得する
	int getErrorCode() const;
	/// @brief エラーコードを設定する
	void setErrorCode(int code);
	/// @brief パーシングが完了しているかどうかを返す
	bool isComplete() const;

	/**
	 * @brief リクエストヘッド（リクエストライン + ヘッダー）をパースする
	 * @param req パース結果を格納するHttpRequestオブジェクト
	 * @param buffer 受信バッファ（パース済み部分は削除される）
	 * @return パース結果（PARSE_INCOMPLETE, PARSE_HEADERS_COMPLETE,
	 * PARSE_ERROR）
	 * @note
	 * このメソッドは内部で状態管理を行い、リクエストラインとヘッダーを順次パースする
	 */
	ParseResult parseHead(HttpRequest &req, std::string &buffer);

	/**
	 * @brief リクエストボディをパースする
	 * @param req
	 * パース結果を格納するHttpRequestオブジェクト（ボディサイズ制限も含む）
	 * @param buffer 受信バッファ（パース済み部分は削除される）
	 * @return パース結果（PARSE_INCOMPLETE, PARSE_COMPLETE, PARSE_ERROR）
	 * @note このメソッドは内部でボディサイズチェックも行う
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

	RequestLineParser _lineParser;
	RequestHeadParser _headParser;
	RequestBodyParser _bodyParser;

	RequestParser(const RequestParser &);
	RequestParser &operator=(const RequestParser &);
};

#endif
