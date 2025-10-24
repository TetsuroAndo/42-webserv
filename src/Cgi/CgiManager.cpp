// src/Cgi/CgiManager.cpp

#include "CgiManager.hpp"
#include "../Config/Config.hpp"
#include "../Handler/HandlerUtil.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "CgiWorker.hpp"
#include <algorithm>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>

void CgiManager::_removeWorker(CgiWorker *worker) {
	if (!worker)
		return;

	LOG(DEBUG) << "Removing CGI worker"
			   << attr("clientFd", worker->getClientFd())
			   << attr("pid", worker->getPid());

	_workers.erase(std::remove(_workers.begin(), _workers.end(), worker),
				   _workers.end());
	_pipeFdToWorker.erase(worker->getReadFd());
	_pipeFdToWorker.erase(worker->getWriteFd());
	_clientFdToWorker.erase(worker->getClientFd());

	delete worker;
}

CgiManager::CgiManager(const Config &config)
	: _timeoutSeconds(config.getTimeoutSec()) {}

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
	try {
		const Location &loc = ctx.conf.getLocation(ctx.req.getPath());
		const std::string scriptPath =
			HandlerUtil::resolvePath(ctx.req.getPath(), ctx.conf);
		std::string interpreterPath;
		const size_t dotPos = scriptPath.rfind('.');

		if (dotPos != std::string::npos) {
			const std::string ext = scriptPath.substr(dotPos);
			if (loc.cgiConf.count(ext)) {
				interpreterPath = loc.cgiConf.at(ext);
			}
		}

		if (interpreterPath.empty()) {
			LOG(WARNING) << "No CGI interpreter found for the request path: "
						 << scriptPath;
			HandlerUtil::generateSimpleBody(ctx.req.getMethod(), ctx.res,
											HttpStatus::NOT_FOUND);
			return;
		}

		LOG(DEBUG) << "Using CGI interpreter"
				   << attr("interpreter", interpreterPath)
				   << attr("script", scriptPath);

		CgiWorker *worker = new CgiWorker(ctx, scriptPath, interpreterPath);
		worker->execute(); // pipe, fork, execveの実行

		_workers.push_back(worker);
		_pipeFdToWorker[worker->getReadFd()] = worker;
		if (worker->getWriteFd() >= 0) {
			_pipeFdToWorker[worker->getWriteFd()] = worker;
		}
		_clientFdToWorker[worker->getClientFd()] = worker;

		// サーバーに監視対象のFDを追加
		// CGIスクリプトからの出力を監視
		{
			FdEventChange ev;
			ev.fd = worker->getReadFd();
			ev.eventType = EPOLLIN;
			ev.changeType = FdChangeType_ADD;
			_queue.push(ev);
		}
		// CGIスクリプトへのリクエストボディの書き込みを監視
		// 書き込みFDが存在する場合のみ登録する（GETなどで不要な場合は-1）
		if (worker->getWriteFd() >= 0) {
			FdEventChange ev;
			ev.fd = worker->getWriteFd();
			ev.eventType = EPOLLOUT;
			ev.changeType = FdChangeType_ADD;
			_queue.push(ev);
		}

		LOG(INFO) << "CGI worker created"
				  << attr("clientFd", worker->getClientFd())
				  << attr("pid", worker->getPid())
				  << attr("readFd", worker->getReadFd())
				  << attr("writeFd", worker->getWriteFd());

	} catch (const std::exception &e) {
		LOG(ERROR) << "Failed to create CGI worker: " << e.what();
		HandlerUtil::generateSimpleBody(ctx.req.getMethod(), ctx.res,
										HttpStatus::INTERNAL_SERVER_ERROR);
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
			ev.changeType = FdChangeType_REMOVE;
			_queue.push(ev);
		}
		_pipeFdToWorker.erase(worker->getWriteFd());
	}

	// CGIプロセスが完了またはエラーになったかチェック
	if (worker->isFinished()) {
		LOG(INFO) << "CGI worker finished"
				  << attr("clientFd", worker->getClientFd())
				  << attr("pid", worker->getPid())
				  << attr("state", worker->getState());
		{
			FdEventChange ev;
			ev.fd = worker->getReadFd();
			ev.changeType = FdChangeType_REMOVE;
			_queue.push(ev);
		}
		// 書き込みFDがまだ監視対象ならそれも削除リストに追加
		if (_pipeFdToWorker.count(worker->getWriteFd())) {
			{
				FdEventChange ev;
				ev.fd = worker->getWriteFd();
				ev.changeType = FdChangeType_REMOVE;
				_queue.push(ev);
			}
		}

		// クライアントFDに対して送信可能イベントを通知して、
		// Server側でisCgiComplete()のチェックをトリガーする
		{
			FdEventChange notify;
			notify.fd = worker->getClientFd();
			notify.eventType = EPOLLIN | EPOLLOUT;
			notify.changeType = FdChangeType_NOTIFY;
			_queue.push(notify);
		}
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

		if (worker->getPid() > 0) {
			kill(worker->getPid(), SIGKILL);
			waitpid(worker->getPid(), NULL, 0);
		}
		worker->setTimeout();

		// 関連FDを監視対象から削除
		{
			FdEventChange ev;
			ev.fd = worker->getReadFd();
			ev.changeType = FdChangeType_REMOVE;
			_queue.push(ev);
		}
		if (_pipeFdToWorker.count(worker->getWriteFd())) {
			{
				FdEventChange ev;
				ev.fd = worker->getWriteFd();
				ev.changeType = FdChangeType_REMOVE;
				_queue.push(ev);
			}
		}
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

FdEventChange CgiManager::popChange() {
	const FdEventChange change = _queue.front();
	_queue.pop();
	return change;
}

size_t CgiManager::eventSize() const { return _queue.size(); }
