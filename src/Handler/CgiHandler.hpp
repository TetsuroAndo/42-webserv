#pragma once

#include "ISubHandler.hpp"
#include "../Cgi/CgiManager.hpp"

class CgiManager;

class CgiHandler : public ISubHandler {
public:
	explicit CgiHandler(CgiManager *cgiManager);
	virtual ~CgiHandler();

	/**
	 * @brief CGI処理を開始するハンドラ
	 * @note このメソッドは即座にレスポンスを返さず、CGIプロセスを起動する。
	 *       レスポンスにはCGI実行中を示す内部的なステータスを設定する。
	 */
	virtual HttpResponse handle(PipelineContext &ctx);

private:
	CgiManager *_cgiManager;

	CgiHandler(const CgiHandler &);
	CgiHandler &operator=(const CgiHandler &);
};
