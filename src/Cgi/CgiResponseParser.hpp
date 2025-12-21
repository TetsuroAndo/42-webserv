// src/Cgi/CgiResponseParser.hpp

#pragma once

#include "../Http/Core/HttpResponse.hpp"
#include <map>
#include <string>

class CgiResponseParser {
public:
	CgiResponseParser();
	~CgiResponseParser();

	/**
	 * @brief CGIレスポンスにヘッダが含まれていたかを判定する
	 * @return ヘッダが見つかった場合はtrue
	 */
	bool headersFound() const;

	/**
	 * @brief CGIからの生レスポンス文字列を解析する
	 * @param rawResponse CGIスクリプトの標準出力から読み取った全データ
	 */
	void parse(const std::string &rawResponse);

	/**
	 * @brief 解析結果をHttpResponseオブジェクトに設定する
	 * @param httpResponse 設定対象のHttpResponseオブジェクト
	 */
	void setResponse(HttpResponse &httpResponse);

	/**
	 * @brief 解析されたHTTPステータスコードを取得する
	 * @return ステータスコード（Status:ヘッダがない場合は200）
	 */
	int getStatusCode() const;

	/**
	 * @brief Status:ヘッダが明示的に設定されたかを判定する
	 * @return Status:ヘッダが設定された場合はtrue
	 */
	bool hasStatusHeader() const;

private:
	int _statusCode;
	std::string
		_statusMessage; // CGIから渡されるStatusMessage現在は使用しない予定
	std::map< std::string, std::string > _headers;
	std::string _body;
	bool _headersParsed;
	bool _hasStatusHeader;

	void _parseHeaders(const std::string &headerBlock);

	CgiResponseParser(const CgiResponseParser &);
	CgiResponseParser &operator=(const CgiResponseParser &);
};
