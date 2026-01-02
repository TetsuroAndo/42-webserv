#pragma once

#include "../Socket/FdEventChanges.hpp"
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
	 * @brief CGIのパイプFDでイベントが発生した際にServerから呼ばれる
	 * @param fd イベントが発生したファイルディスクリプタ
	 * @param event_type イベントのタイプ (EPOLLIN or EPOLLOUT)
	 */
	void handleEvent(int fd, uint32_t event_type); // TODO: AEventにする

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
	 * @brief 指定されたFDがCgiManagerの管理下にあるか判定する
	 * @param fd 判定対象のファイルディスクリプタ
	 * @return 管理下にあればtrue
	 */
	bool isCgiFd(int fd) const;

	/**
	 * @brief 指定クライアントFDに紐づくCGIを中断・後始末する
	 */
	void abortClient(int clientFd);

	/**
	 * @brief _addから情報を一個取り出す
	 * @return _addの一番先頭の要素
	 */
	FdEventChange popAddChange();

	/**
	 * @brief _deleteから情報を一個取り出す
	 * @return _deleteの一番先頭の要素
	 */
	FdEventChange popRemoveChange();

	/**
	 * @brief 残っているaddのFdEventChangesの数を返す
	 * @return 残っているaddのFdEventChangesの数
	 */
	size_t sizeAddEvent() const;
	/**
	 * @brief 残っているdeleteのFdEventChangesの数を返す
	 * @return 残っているdeleteのFdEventChangesの数
	 */
	size_t sizeRemoveEvent() const;

	/**
	 * @brief 完了通知（クライアントFD）を一件取り出す
	 * @return 完了したクライアントFD
	 */
	int popCompletedClientFd();

	/**
	 * @brief 残っている完了通知の数を返す
	 * @return 残っている完了通知の数
	 */
	size_t sizeCompletedClientFd() const;

private:
	const time_t _timeoutSeconds;
	size_t _maxWorkers;
	std::vector< CgiWorker * > _workers;
	// pipeFDからWorkerを引くためのマップ
	std::map< int, CgiWorker * > _pipeFdToWorker;
	// ClientFDからWorkerを引くためのマップ
	std::map< int, CgiWorker * > _clientFdToWorker;
	// PIDからWorkerを引くためのマップ
	std::map< pid_t, CgiWorker * > _pidToWorker;

	// FdEventChangesを貯めるキュー
	std::queue< FdEventChange > _add;
	std::queue< FdEventChange > _remove;

	// 完了・エラー・タイムアウトしたCGIに紐づくクライアントFD通知キュー
	std::queue< int > _completedClients;

	void _removeWorker(CgiWorker *worker);

	CgiManager(const CgiManager &);
	CgiManager &operator=(const CgiManager &);
};
