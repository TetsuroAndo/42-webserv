#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include "CgiResponseParser.hpp"
#include <string>
#include <sys/time.h>
#include <vector>

class CgiManager;

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
			  const std::string &interpreterPath, CgiManager *manager);
	~CgiWorker();

	/// @brief CGIプロセスをfork/execveで実行する
	void execute();

	/// @brief CGIプロセスの標準入力へリクエストボディを書き込む
	void handleWrite();

	/// @brief CGIプロセスの標準出力からレスポンスを読み込む
	void handleRead();

	/// @brief CGIプロセスの標準エラー出力からエラーメッセージを読み込む
	void handleReadErr();

	/// @brief CGIプロセスが失敗した際の処理
	void handleErrorExit();

	/// @brief クライアントFDを取得する
	int getClientFd() const;

	/// @brief 読み込み用パイプのFDを取得する
	int getReadFd() const;

	/// @brief 書き込み用パイプのFDを取得する
	int getWriteFd() const;

	/// @brief 標準エラー出力読み込み用パイプのFDを取得する
	int getErrFd() const;

	/// @brief CGI起動ステータス読み込み用パイプのFDを取得する
	int getStatusFd() const;

	/// @brief CGIが完了したイベントを受け取るためのFDを取得する
	int getCompletionFdOut() const;

	/// @brief CGIが完了したイベントを発火するためのFDを取得する
	int getCompletionFdIn() const;

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

	/// @brief CGIタイムアウト秒数を取得する
	time_t getTimeoutSeconds() const;

	/// @brief 最終活動時刻を更新する
	void updateLastActivityTime();

	/// @brief タイムアウトしたか判定する
	bool isTimeout() const;

	/// @brief 処理が完了したか（正常・エラー・タイムアウト問わず）
	bool isFinished() const;

	/// @brief CGIの実行結果をHttpResponseオブジェクトに設定する
	void createHttpResponse(HttpResponse &res);

	/// @brief プロセスの終了ステータスを設定する
	void setExitStatus(int status);

	/// @brief プロセスの終了ステータスを取得する
	int getExitStatus() const;

	/// @brief Status:ヘッダが明示的に設定されたかを判定する
	bool hasStatusHeader() const;

	/// @brief 終了ステータスが設定されたかを判定する
	bool isExitStatusSet() const;

	/// @brief 紐ずくCgiManagerを取得する
	CgiManager *getManager() const;

	/// @brief 通知済みかを取得する
	bool isCompletionNotified() const;

	/// @brief 通知済みにする
	void setCompletionNotified();

	/// @brief EventManager/epollから関連FDを外す
	void detachEvents(bool keepCompletionEvent) const;

	/// @brief completion fdのdetachをスキップするか設定する
	void setKeepCompletionEventOnDetach(bool keepCompletionEvent);

	/// @brief EventManagerに登録したFDを記録する
	void setEventFds(int readFd, int writeFd, int errFd, int statusFd,
					 int completionFd);

private:
	PipelineContext &_ctx;
	CgiManager *_manager;
	CgiState _state;
	int _clientFd;
	pid_t _pid;
	int _exitStatus;
	bool _exitStatusSet;
	bool _outputComplete;
	bool _completionNotified;
	int _pipeIn[2];
	int _pipeOut[2];
	int _pipeErr[2];
	int _pipeStatus[2];
	int _pipeComplete[2];
	std::string _requestBody;
	size_t _bytesSent;
	std::string _scriptPath;
	std::string _interpreterPath;
	time_t _lastActivityTime;
	time_t _timeoutSeconds;
	std::string _responseBuffer;
	std::vector< char > _readBuffer;
	std::vector< char > _errBuffer;

	CgiResponseParser _responseParser;
	int _eventReadFd;
	int _eventWriteFd;
	int _eventErrFd;
	int _eventStatusFd;
	int _eventCompletionFd;
	bool _keepCompletionEventOnDetach;

	void _childProcess(const std::string &scriptPath,
					   const std::string &interpreterPath,
					   const std::vector< std::string > &envpStrs) const;
	static void _closePipe(int &fd);

	CgiWorker(const CgiWorker &);
	CgiWorker &operator=(const CgiWorker &);
};
