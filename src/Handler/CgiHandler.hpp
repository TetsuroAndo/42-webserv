#pragma once

#include "../Cgi/CgiManager.hpp"
#include "ISubHandler.hpp"

class CgiManager;

class CgiHandler : public ISubHandler {
public:
	explicit CgiHandler();
	~CgiHandler();

	/**
	 * @brief CGI処理を開始するハンドラ
	 * @note このメソッドは即座にレスポンスを返さず、CGIプロセスを起動する。
	 *       レスポンスにはCGI実行中を示す内部的なステータスを設定する。
	 */
	virtual HttpResponse handle(PipelineContext &ctx);

private:
	CgiHandler(const CgiHandler &);
	CgiHandler &operator=(const CgiHandler &);
};
