#pragma once

#include "../Middleware/Core/PipelineContext.hpp"
#include "../Socket/FdEventChanges.hpp"
#include "CgiWorker.hpp"
#include <map>
#include <vector>

class CgiManager {
public:
	CgiManager(const Config &config);
	~CgiManager();

	/**
	 * @brief 新しいCgiWorkerを生成し、監視対象のFDリストを返す
	 * @param ctx リクエストのコンテキスト
	 * @return Serverのepollに追加・削除すべきFDの情報
	 */
	FdEventChanges createWorker(PipelineContext &ctx);

	/**
	 * @brief CGIのパイプFDでイベントが発生した際にServerから呼ばれる
	 * @param fd イベントが発生したファイルディスクリプタ
	 * @return 状態変化によりepollへの登録内容を変更するための情報
	 */
	FdEventChanges handleEvent(int fd);

	/**
	 * @brief 完了またはタイムアウトしたWorkerをクリーンアップする
	 * @return epollから削除すべきFDの情報
	 */
	FdEventChanges cleanupWorkers();

	/**
	 * @brief タイムアウトしたWorkerをクリーンアップする
	 * @return epollから削除すべきFDの情報
	 */
	FdEventChanges cleanupTimedOutWorkers();

	/**
	 * @brief 指定したクライアント向けのCGI処理が完了したか確認する
	 * @param clientFd クライアントのファイルディスクリプタ
	 * @param res
	 * 処理が完了していた場合、このHttpResponseオブジェクトに結果が格納される
	 * @return 処理が完了していればtrue、そうでなければfalse
	 */
	bool isCgiComplete(int clientFd, HttpResponse &res);

	/**
	 * @brief 指定されたFDがCgiManagerの管理下にあるか判定する
	 * @param fd 判定対象のファイルディスクリプタ
	 * @return 管理下にあればtrue
	 */
	bool isCgiFd(int fd) const;

private:
	const time_t _timeoutSeconds;
	std::vector< CgiWorker * > _workers;
	// パイプFDからWorkerを引くためのマップ
	std::map< int, CgiWorker * > _pipeFdToWorker;
	// ClientFDからWorkerを引くためのマップ
	std::map< int, CgiWorker * > _clientFdToWorker;

	void _removeWorker(CgiWorker *worker);

	CgiManager(const CgiManager &);
	CgiManager &operator=(const CgiManager &);
};
