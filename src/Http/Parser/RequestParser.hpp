#ifndef HTTP_REQUEST_HELPER_HPP
#define HTTP_REQUEST_HELPER_HPP

#include "../Core/HttpRequest.hpp"
#include "ParseResult.hpp"
#include "RequestBodyParser.hpp"
#include "RequestHeaderParser.hpp"
#include "RequestLineParser.hpp"
#include <string>

class RequestParser {
public:
	RequestParser();
	~RequestParser();

	void reset();

	/**
	 * @brief PARSE_ERRORの場合のHTTPステータスコードを返します。
	 * @return int エラーに対応するHTTPステータスコード (e.g., 400, 413)。
	 */
	int getErrorCode() const;

	/**
	 * @brief パースが完了しているかどうかを返します。
	 * @return bool パースが完了していればtrue、そうでなければfalse。
	 */
	bool isComplete() const;

	/**
	 * @brief 生のリクエストバッファをパースし、HttpRequestオブジェクトを構築します。
	 * @param request 構築対象のHttpRequestオブジェクト。
	 * @param buffer 受信した生データが入ったバッファ。パースした分は削除されます。
	 * @return ParseResult パース結果。
	 */
	ParseResult parse(HttpRequest& request, std::string& buffer);

private:
	enum ParseState {
		STATE_REQUEST_LINE,
		STATE_HEADERS,
		STATE_BODY,
		STATE_COMPLETE
	};

	int _errorCode;
	ParseState _state;

	RequestLineParser _lineParser;
	RequestHeaderParser _headerParser;
	RequestBodyParser _bodyParser;

	RequestParser(const RequestParser&);
	RequestParser& operator=(const RequestParser&);
};

#endif
