#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include "CgiResponseParser.hpp"
#include <string>
#include <sys/time.h>
#include <vector>

class CgiWorker {
public:
	enum CgiState {
		CGI_INIT,		  // 初期状態
		CGI_SENDING_BODY, // リクエストボディをCGIに送信中
		CGI_RECEIVING,	  // CGIからのレスポンスを受信中
		CGI_COMPLETE,	  // 正常完了
		CGI_ERROR,		  // 内部エラー発生
		CGI_TIMEOUT		  // タイムアウト発生
	};

	CgiWorker(PipelineContext &ctx, const std::string &scriptPath,
			  const std::string &interpreterPath);
	~CgiWorker();

	/// @brief CGIプロセスをfork/execveで実行する
	void execute();

	/// @brief CGIプロセスの標準入力へリクエストボディを書き込む
	void handleWrite();

	/// @brief CGIプロセスの標準出力からレスポンスを読み込む
	void handleRead();

	/// @brief CGIプロセスの標準エラー出力からエラーメッセージを読み込む
	void handleReadErr();

	/// @brief クライアントFDを取得する
	int getClientFd() const;

	/// @brief 読み込み用パイプのFDを取得する
	int getReadFd() const;

	/// @brief 書き込み用パイプのFDを取得する
	int getWriteFd() const;

	/// @brief 標準エラー出力読み込み用パイプのFDを取得する
	int getErrFd() const;

	/// @brief 子プロセスのPIDを取得する
	pid_t getPid() const;

	/// @brief 現在の状態を取得する
	CgiState getState() const;

	/// @brief 状態をタイムアウトに設定する
	void setTimeout();

	/// @brief 状態をエラーに設定する
	void setError();

	/// @brief 最終活動時刻を取得する
	time_t getLastActivityTime() const;

	/// @brief 最終活動時刻を更新する
	void updateLastActivityTime();

	/// @brief タイムアウトしたか判定する
	bool isTimeout() const;

	/// @brief 処理が完了したか（正常・エラー・タイムアウト問わず）
	bool isFinished() const;

	/// @brief CGIの実行結果をHttpResponseオブジェクトに設定する
	void createHttpResponse(HttpResponse &res);

private:
	PipelineContext &_ctx;
	CgiState _state;
	int _clientFd;
	pid_t _pid;
	int _pipeIn[2];
	int _pipeOut[2];
	int _pipeErr[2];
	std::string _requestBody;
	size_t _bytesSent;
	std::string _scriptPath;
	std::string _interpreterPath;
	time_t _lastActivityTime;
	std::string _responseBuffer;
	std::vector< char > _readBuffer;
	std::vector< char > _errBuffer;

	CgiResponseParser _responseParser;

	void _childProcess(const std::string &scriptPath,
					   const std::string &interpreterPath,
					   const std::vector< std::string > &envpStrs) const;
	static void _closePipe(int &fd);
	void _updateLastActivityTime();

	CgiWorker(const CgiWorker &);
	CgiWorker &operator=(const CgiWorker &);
};
