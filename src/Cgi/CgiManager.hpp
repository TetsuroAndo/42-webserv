#pragma once

#include "../Socket/FdEventChanges.hpp"
#include "CgiWorker.hpp"
#include <map>
#include <string>
#include <vector>

/*
 * CgiManagerクラス (Singleton)
 *
 * 全てのCgiWorkerのライフサイクルを管理する。
 * Workerの生成、イベントのディスパッチ、完了したWorkerのクリーンアップを行う。
 */
class CgiManager {
public:
	CgiManager();
	~CgiManager();

	/**
	 * @brief 新しいCgiWorkerを生成し、管理下に置く
	 * CgiWorkerの読み書き用ファイルディスクリプタをSocketsManagerに登録する
	 * Client -> CgiHandlerから呼び出される
	 */
	FdEventChanges createWorker(int clientFd, const HttpRequest &req, const Location &locConf,
					  const std::string &scriptPath, const std::string &interpreterPath);
	FdEventChanges handleEvent(int fd);
	FdEventChanges cleanupWorkers();

	bool isCgiComplete(int clientFd, HttpResponse &res);

private:
	std::vector<CgiWorker*> _workers;
	std::map<int, CgiWorker*> _fdToWorker;
	std::map<int, CgiWorker*> _clientFdToWorker;

	void _removeWorker(CgiWorker* worker);

	CgiManager();
	CgiManager(const CgiManager&);
	CgiManager &operator=(const CgiManager&);
};
