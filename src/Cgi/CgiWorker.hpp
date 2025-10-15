#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include "CgiResponseParser.hpp"
#include <string>
#include <sys/time.h>
#include <unistd.h>
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

	/// @brief 読み込み用パイプのFDを取得する
	int getReadFd() const;

	/// @brief 書き込み用パイプのFDを取得する
	int getWriteFd() const;

	/// @brief 子プロセスのPIDを取得する
	pid_t getPid() const;

	/// @brief 現在の状態を取得する
	CgiState getState() const;

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
	pid_t _pid;
	int _pipeIn[2];
	int _pipeOut[2];
	CgiState _state;
	std::string _requestBody;
	size_t _bytesSent;
	std::string _responseBuffer;
	time_t _lastActivityTime;

	CgiResponseParser _responseParser;

	void _childProcess(const std::string &scriptPath,
					   const std::string &interpreterPath,
					   const std::vector< std::string > &envp_strs);
	void _closePipe(int &fd);
	void _updateLastActivityTime();

	CgiWorker(const CgiWorker &);
	CgiWorker &operator=(const CgiWorker &);
};
