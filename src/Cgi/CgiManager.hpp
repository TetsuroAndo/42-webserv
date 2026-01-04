#pragma once

#include <bits/stdint-uintn.h>
#include <cstddef>
#include <ctime>
#include <map>
#include <queue>
#include <vector>

class Config;
struct PipelineContext;
class HttpResponse;
class CgiWorker;

class CgiWriteEvent;
class CgiReadEvent;
class CgiErrorEvent;

class CgiManager {
public:
	CgiManager(const Config &config);
	~CgiManager();

	/**
	 * @brief 新しいCgiWorkerを生成し、監視対象のFDリストを返す
	 * @param ctx リクエストのコンテキスト
	 */
	void createWorker(PipelineContext &ctx);

	/**
	 * @brief タイムアウトしたWorkerをクリーンアップする
	 */
	void cleanupTimedOutWorkers();

	/**
	 * @brief 終了した全てのCGIプロセスを非ブロッキングで回収する（ゾンビ防止）
	 */
	void cleanupFinishedWorkers();

	/**
	 * @brief 指定したクライアント向けのCGI処理が完了したか確認する
	 * @param clientFd クライアントのファイルディスクリプタ
	 * @param res
	 * ステータス関係なく完了していればHttpResponseを構築してワーカーを削除する
	 * @return 処理が完了していればtrue、そうでなければfalse
	 */
	bool isCgiComplete(int clientFd, HttpResponse &res);

	/**
	 * @brief 指定クライアントFDに紐づくCGIを中断・後始末する
	 */
	void abortClient(int clientFd);

private:
	const time_t _timeoutSeconds;
	size_t _maxWorkers;
	std::vector< CgiWorker * > _workers;
	// ClientFDからWorkerを引くためのマップ
	std::map< int, CgiWorker * > _clientFdToWorker;
	// PIDからWorkerを引くためのマップ
	std::map< pid_t, CgiWorker * > _pidToWorker;

	// 完了・エラー・タイムアウトしたCGIに紐づくクライアントFD通知キュー
	std::queue< int > _completedClients;

	void _removeWorker(CgiWorker *worker);

	CgiManager(const CgiManager &);
	CgiManager &operator=(const CgiManager &);
};
