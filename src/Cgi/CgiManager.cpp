#include "CgiManager.hpp"
#include "../Config/Config.hpp"
#include "../Http/Resolver/RequestResolver.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "CgiWorker.hpp"
#include <algorithm>
#include <cstring>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void CgiManager::_removeWorker(CgiWorker *worker) {
	if (!worker)
		return;

	LOG(DEBUG) << "Removing CGI worker"
			   << attr("clientFd", worker->getClientFd())
			   << attr("pid", worker->getPid());

	_workers.erase(std::remove(_workers.begin(), _workers.end(), worker),
				   _workers.end());
	// 値がworkerのエントリをすべて削除する（fdが-1に変更された後でも安全）
	{
		std::vector< int > keys;
		for (std::map< int, CgiWorker * >::iterator it2 =
				 _pipeFdToWorker.begin();
			 it2 != _pipeFdToWorker.end(); ++it2) {
			if (it2->second == worker)
				keys.push_back(it2->first);
		}
		for (size_t i = 0; i < keys.size(); ++i) {
			_pipeFdToWorker.erase(keys[i]);
		}
	}
	_clientFdToWorker.erase(worker->getClientFd());
	if (worker->getPid() > 0) {
		_pidToWorker.erase(worker->getPid());
	}

	delete worker;
}

CgiManager::CgiManager(const Config &c)
	: _timeoutSeconds(c.getTimeoutSec()),
	  _maxWorkers(std::max< size_t >(
		  c.getPerformance().cgiMinWorkers,
		  std::min< size_t >(c.getMaxEvents(),
							 c.getPerformance().cgiMaxWorkers))) {}

CgiManager::~CgiManager() {
	std::vector< CgiWorker * >::iterator it = _workers.begin();
	for (; it != _workers.end(); ++it) {
		if ((*it)->getPid() > 0) {
			kill((*it)->getPid(), SIGKILL);
			waitpid((*it)->getPid(), NULL, 0);
		}
		delete *it;
	}
	_workers.clear();
}

void CgiManager::createWorker(PipelineContext &ctx) {
	// catch ブロックで delete できるように try の外で宣言
	CgiWorker *worker = NULL;
	try {
		if (_workers.size() >= _maxWorkers) {
			LOG(WARNING) << "CGI worker limit reached"
						 << attr("limit", _maxWorkers);
			ctx.res.setStatusCode(HttpStatus::SERVICE_UNAVAILABLE);
			return;
		}
		const Location &loc = ctx.conf.getLocation(ctx.req.getPath());

		// リクエストからスクリプト仮想パスとPATH_INFOを切り出す
		std::string scriptVirtual;
		std::string pathInfo;
		if (!RequestResolver::extractCgiScript(ctx.req.getPath(), loc,
										   scriptVirtual, pathInfo)) {
			LOG(WARNING) << "Failed to extract CGI script from request"
						 << attr("path", ctx.req.getPath());
			ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
			return;
		}

		// スクリプトの実ファイル（絶対パス）を解決（PATH_INFOは含めない）
		const std::string scriptPath =
			RequestResolver::resolvePath(scriptVirtual, ctx.conf);

		if (scriptPath.empty()) {
			LOG(WARNING) << "CGI script not found"
						 << attr("scriptVirtual", scriptVirtual);
			ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
			return;
		}

		// インタプリタの解決（拡張子は scriptVirtual から）
		std::string interpreterPath;
		const size_t dotPosVirtual = scriptVirtual.rfind('.');
		if (dotPosVirtual != std::string::npos) {
			const std::string ext = scriptVirtual.substr(dotPosVirtual);
			if (loc.cgiConf.count(ext)) {
				interpreterPath = loc.cgiConf.at(ext);
			}
		}

		if (interpreterPath.empty()) {
			LOG(WARNING) << "No CGI interpreter found for the request path: "
						 << scriptPath;
			ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
			return;
		}

		LOG(DEBUG) << "Using CGI interpreter"
				   << attr("interpreter", interpreterPath)
				   << attr("script", scriptPath);

		worker = new CgiWorker(ctx, scriptPath, interpreterPath);
		worker->execute(); // pipe, fork, execveの実行

		_workers.push_back(worker);
		if (worker->getReadFd() >= 0) {
			_pipeFdToWorker[worker->getReadFd()] = worker;
		}
		if (worker->getWriteFd() >= 0) {
			_pipeFdToWorker[worker->getWriteFd()] = worker;
		}
		_clientFdToWorker[worker->getClientFd()] = worker;
		if (worker->getPid() > 0) {
			_pidToWorker[worker->getPid()] = worker;
		}

		// サーバーに監視対象のFDを追加
		// CGIスクリプトからの出力を監視
		if (worker->getReadFd() >= 0) {
			FdEventChange ev;
			ev.fd = worker->getReadFd();
			ev.eventType = EPOLLIN;
			_add.push(ev);
		}
		// CGIスクリプトへのリクエストボディの書き込みを監視（fdが有効な場合）
		if (worker->getWriteFd() >= 0) {
			FdEventChange ev;
			ev.fd = worker->getWriteFd();
			ev.eventType = EPOLLOUT;
			_add.push(ev);
		}

		LOG(INFO) << "CGI worker created"
				  << attr("clientFd", worker->getClientFd())
				  << attr("pid", worker->getPid())
				  << attr("readFd", worker->getReadFd())
				  << attr("writeFd", worker->getWriteFd());

	} catch (const std::exception &e) {
		// new または execute で失敗した場合に備えて delete (NULLでも問題なし)
		delete worker;
		LOG(ERROR) << "Failed to create CGI worker: " << e.what();
		ctx.res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
	}
}

void CgiManager::handleEvent(const int fd, const uint32_t event_type) {
	std::map< int, CgiWorker * >::iterator it = _pipeFdToWorker.find(fd);
	if (it == _pipeFdToWorker.end()) {
		return;
	}

	CgiWorker *worker = it->second;
	worker->updateLastActivityTime();

	if (event_type & EPOLLIN) { // CGIからの読み込み可能
		worker->handleRead();
	}
	if (event_type & EPOLLOUT) { // CGIへの書き込み可能
		worker->handleWrite();
	}

	// 書き込みが完了したら、書き込みFDの監視を解除
	if (worker->getState() == CgiWorker::CGI_RECEIVING) {
		if (worker->getWriteFd() >= 0) {
			FdEventChange ev;
			ev.fd = worker->getWriteFd();
			_remove.push(ev);
		}
		_pipeFdToWorker.erase(worker->getWriteFd());
	}

	// CGIプロセスが完了またはエラーになったかチェック
	if (worker->isFinished()) {
		LOG(INFO) << "CGI worker finished"
				  << attr("clientFd", worker->getClientFd())
				  << attr("pid", worker->getPid())
				  << attr("state", worker->getState());
		// このイベントを発火させたfdを確実に削除
		{
			FdEventChange ev;
			ev.fd = fd;
			_remove.push(ev);
		}
		_pipeFdToWorker.erase(fd);
		// 書き込みFDがまだ監視対象ならそれも削除リストに追加
		if (_pipeFdToWorker.count(worker->getWriteFd())) {
			{
				FdEventChange ev;
				ev.fd = worker->getWriteFd();
				_remove.push(ev);
			}
			_pipeFdToWorker.erase(worker->getWriteFd());
		}
		// 完了したクライアントFDを通知キューに積む
		_completedClients.push(worker->getClientFd());
	}
}

void CgiManager::cleanupTimedOutWorkers() {
	const time_t now = time(NULL);
	std::vector< CgiWorker * > workersToCleanup;

	for (std::vector< CgiWorker * >::iterator it = _workers.begin();
		 it != _workers.end(); ++it) {
		if (now - (*it)->getLastActivityTime() > _timeoutSeconds) {
			workersToCleanup.push_back(*it);
		}
	}

	for (std::vector< CgiWorker * >::iterator it = workersToCleanup.begin();
		 it != workersToCleanup.end(); ++it) {
		CgiWorker *worker = *it;
		LOG(WARNING) << "CGI worker timed out. Killing process."
					 << attr("clientFd", worker->getClientFd())
					 << attr("pid", worker->getPid());

		pid_t pid = worker->getPid();
		if (pid > 0) {
			kill(pid, SIGKILL);
			waitpid(pid, NULL, WNOHANG);
			_pidToWorker.erase(pid);
		}
		worker->setTimeout();

		// 完了通知を積む（タイムアウト）
		_completedClients.push(worker->getClientFd());

		// 関連FDを監視対象から削除（現在のマッピングに基づいて安全に）
		{
			std::vector< int > keys;
			for (std::map< int, CgiWorker * >::iterator it2 =
					 _pipeFdToWorker.begin();
				 it2 != _pipeFdToWorker.end(); ++it2) {
				if (it2->second == worker)
					keys.push_back(it2->first);
			}
			for (size_t i = 0; i < keys.size(); ++i) {
				FdEventChange ev;
				ev.fd = keys[i];
				_remove.push(ev);
				_pipeFdToWorker.erase(keys[i]);
			}
		}
	}
}

void CgiManager::cleanupFinishedWorkers() {
	int status;
	pid_t pid;

	// waitpid(-1, ...) は「任意の子プロセス」を待つ。
	// WNOHANG は「ブロッキングしない」オプション。
	// 終了した子プロセスがいなくなるまでループで呼び出す。
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

		// 回収したPIDに対応する CgiWorker を探す
		CgiWorker *worker = NULL;
		std::map< pid_t, CgiWorker * >::iterator pit = _pidToWorker.find(pid);
		if (pit != _pidToWorker.end()) {
			worker = pit->second;
		}

		if (worker) {
			// 管理下のワーカーが終了した
			LOG(DEBUG) << "CGI process reaped by cleanupFinishedWorkers"
					   << attr("pid", pid) << attr("status", status);

			_pidToWorker.erase(pid);

			// ワーカーがまだ終了状態 (COMPLETE, ERROR, TIMEOUT)
			// になっていなければ (例:
			// パイプEOFより先にプロセスがクラッシュした)
			// 強制的に終了処理を行う。
			if (!worker->isFinished()) {
				LOG(WARNING)
					<< "CGI process exited unexpectedly (reaped by manager)"
					<< attr("pid", pid);

				// パイプFDをepollから削除するようキューに入れる
				if (worker->getReadFd() >= 0) {
					FdEventChange ev;
					ev.fd = worker->getReadFd();
					_remove.push(ev);
				}
				if (worker->getWriteFd() >= 0) {
					FdEventChange ev;
					ev.fd = worker->getWriteFd();
					_remove.push(ev);
				}
				worker->setError();
				// 予期せぬ終了を通知
				_completedClients.push(worker->getClientFd());
			}
		} else {
			// _pidToWorker リストにないPID = おそらく handleRead の
			// EOF処理などで既に isCgiComplete() -> _removeWorker()
			// が完了したプロセス。
			LOG(DEBUG) << "Reaped zombie process (PID not in active workers "
					   << "list, likely already handled): " << pid;
		}
	}

	// ECHILD は「待つべき子プロセスがいない」という正常値
	if (pid < 0 && errno != ECHILD) {
		LOG(ERROR) << "waitpid() failed in CgiManager::cleanupFinishedWorkers: "
				   << strerror(errno);
	}
}

bool CgiManager::isCgiComplete(int clientFd, HttpResponse &res) {
	std::map< int, CgiWorker * >::iterator it =
		_clientFdToWorker.find(clientFd);
	if (it == _clientFdToWorker.end()) {
		return false; // CGIリクエストではない
	}

	CgiWorker *worker = it->second;
	if (!worker->isFinished()) {
		return false;
	}

	// CGIの状態に基づいてHTTPレスポンスを生成
	if (worker->getState() == CgiWorker::CGI_COMPLETE) {
		worker->createHttpResponse(res);
	} else if (worker->getState() == CgiWorker::CGI_TIMEOUT) {
		res.setStatusCode(HttpStatus::GATEWAY_TIMEOUT);
	} else { // CGI_ERROR
		res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
	}

	_removeWorker(worker);
	return true;
}

bool CgiManager::isCgiFd(const int fd) const {
	return _pipeFdToWorker.count(fd) > 0;
}

void CgiManager::abortClient(const int clientFd) {
	std::map< int, CgiWorker * >::iterator it =
		_clientFdToWorker.find(clientFd);
	if (it == _clientFdToWorker.end())
		return;

	CgiWorker *worker = it->second;
	// epoll監視から外す（現在のマッピングに基づく）
	{
		std::vector< int > keys;
		for (std::map< int, CgiWorker * >::iterator it2 =
				 _pipeFdToWorker.begin();
			 it2 != _pipeFdToWorker.end(); ++it2) {
			if (it2->second == worker)
				keys.push_back(it2->first);
		}
		for (size_t i = 0; i < keys.size(); ++i) {
			FdEventChange ev;
			ev.fd = keys[i];
			_remove.push(ev);
			_pipeFdToWorker.erase(keys[i]);
		}
	}
	// プロセスを確実に終了
	pid_t pid = worker->getPid();
	if (pid > 0) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, WNOHANG);
		_pidToWorker.erase(pid);
	}
	_removeWorker(worker);
}

FdEventChange CgiManager::popAddChange() {
	const FdEventChange change = _add.front();
	_add.pop();
	return change;
}

FdEventChange CgiManager::popRemoveChange() {
	const FdEventChange change = _remove.front();
	_remove.pop();
	return change;
}

FdEventChange CgiManager::popNotifyChange() {
	const FdEventChange change = _notify.front();
	_notify.pop();
	return change;
}

size_t CgiManager::sizeAddEvent() const { return _add.size(); }

size_t CgiManager::sizeRemoveEvent() const { return _remove.size(); }

size_t CgiManager::sizeNotifyEvent() const { return _notify.size(); }

int CgiManager::popCompletedClientFd() {
	const int cfd = _completedClients.front();
	_completedClients.pop();
	return cfd;
}

size_t CgiManager::sizeCompletedClientFd() const {
	return _completedClients.size();
}
