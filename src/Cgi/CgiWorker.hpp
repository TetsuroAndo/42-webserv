#pragma once


#include "../Config/Config.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include <string>

// clang-format off

enum CgiState {
	CGI_SENDING_BODY,      // リクエストボディをCGIに送信中
	CGI_RECEIVING_HEADERS, // CGIからレスポンスヘッダを受信中
	CGI_RECEIVING_BODY,    // CGIからレスポンスボディを受信中
	CGI_COMPLETE,          // 処理完了
	CGI_ERROR              // エラー発生
};

class CgiWorker {
public:
	CgiWorker(const HttpRequest &req, const Location &locConf, const std::string &scriptPath);
	~CgiWorker();
private:
	const HttpRequest _req;
	const Location _locConf;
	const std::string _scriptPath;

	char **_envp;
};
