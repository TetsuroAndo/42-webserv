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

	/// @brief タイムアウトしたか判定する
	bool isTimeout() const;

	/// @brief 処理が完了したか（正常・エラー・タイムアウト問わず）
	bool isFinished() const;

	/// @brief CGIの実行結果をHttpResponseオブジェクトに設定する
	void createHttpResponse(HttpResponse &res);

private:
	pid_t _pid;
	int _pipe_in[2];
	int _pipe_out[2];
	CgiState _state;
	std::string _request_body;
	size_t _bytes_sent;
	std::string _response_buffer;
	time_t _last_activity_time;

	CgiResponseParser _response_parser;

	static const int TIMEOUT_SECONDS = 30;

	void _childProcess(const std::string &scriptPath,
					   const std::string &interpreterPath,
					   const std::vector< std::string > &envp_strs);
	void _closePipe(int &fd);
	void _updateLastActivityTime();

	CgiWorker(const CgiWorker &);
	CgiWorker &operator=(const CgiWorker &);
};
