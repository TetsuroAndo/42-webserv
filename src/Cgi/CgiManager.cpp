// src/Cgi/CgiManager.cpp

#include "CgiManager.hpp"
#include "../Handler/HandlerUtil.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Lib/Logger/Log.hpp"
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

FdEventChanges CgiManager::createWorker(PipelineContext &ctx) {
	FdEventChanges changes;
	try {
		const Location &loc = ctx.conf.getLocation(ctx.req.getPath());

		// パスを解決（StaticFileHandlerと同じロジック）
		std::string scriptPath =
			HandlerUtil::resolvePath(ctx.req.getPath(), ctx.conf);
		if (scriptPath.empty()) {
			LOG(WARNING) << "No matching location found for CGI request"
						 << attr("path", ctx.req.getPath());
			HandlerUtil::generateSimpleBody(ctx.req.getMethod(), ctx.res,
											HttpStatus::NOT_FOUND);
			return changes;
		}

		LOG(DEBUG) << "Resolved CGI script path"
				   << attr("scriptPath", scriptPath);

		std::string interpreterPath;
		size_t dotPos = scriptPath.rfind('.');
		if (dotPos != std::string::npos) {
			std::string ext = scriptPath.substr(dotPos);
			if (loc.cgiConf.count(ext)) {
				interpreterPath = loc.cgiConf.at(ext);
			}
		}

		if (interpreterPath.empty()) {
			LOG(WARNING) << "No CGI interpreter found for the request path: "
						 << scriptPath;
			HandlerUtil::generateSimpleBody(ctx.req.getMethod(), ctx.res,
											HttpStatus::NOT_FOUND);
			return changes;
		}

		LOG(DEBUG) << "Using CGI interpreter"
				   << attr("interpreter", interpreterPath)
				   << attr("script", scriptPath);

		CgiWorker *worker = new CgiWorker(ctx, scriptPath, interpreterPath);
		worker->execute(); // pipe, fork, execveの実行

		_workers.push_back(worker);
		_pipeFdToWorker[worker->getReadFd()] = worker;
		_clientFdToWorker[worker->getClientFd()] = worker;

		FdEvent readEvent;
		readEvent.fd = worker->getReadFd();
		readEvent.event_type = EPOLLIN;
		changes.fdsToAdd.push_back(readEvent);

		// Only add write FD if we have a body to send
		if (worker->getWriteFd() >= 0) {
			_pipeFdToWorker[worker->getWriteFd()] = worker;
			FdEvent writeEvent;
			writeEvent.fd = worker->getWriteFd();
			writeEvent.event_type = EPOLLOUT;
			changes.fdsToAdd.push_back(writeEvent);
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
	return changes;
}

FdEventChanges CgiManager::handleEvent(int fd, uint32_t eventType) {
	FdEventChanges changes;

	std::map< int, CgiWorker * >::iterator it = _pipeFdToWorker.find(fd);
	if (it == _pipeFdToWorker.end()) {
		LOG(WARNING) << "FD not found in _pipeFdToWorker" << attr("fd", fd);
		return changes;
	}

	CgiWorker *worker = it->second;
	worker->updateLastActivityTime();

	// EPOLLHUPやEPOLLERRが発生した場合も、データを読み切る
	if (eventType & EPOLLIN) {
		worker->handleRead();
	}
	if (eventType & EPOLLOUT) {
		worker->handleWrite();
	}

	// EPOLLHUPの場合、パイプが閉じられたので読み取りを試みる
	if (eventType & EPOLLHUP) {
		worker->handleRead();
	}

	// 書き込みが完了したら、書き込みFDの監視を解除
	if (worker->getState() == CgiWorker::CGI_RECEIVING) {
		// writeFdが有効で、かつまだmapに存在する場合のみ削除
		int writeFd = worker->getWriteFd();
		if (writeFd >= 0 && _pipeFdToWorker.count(writeFd)) {
			changes.fdsToRemove.push_back(writeFd);
			_pipeFdToWorker.erase(writeFd);
		}
	}

	// CGIプロセスが完了またはエラーになったかチェック
	if (worker->isFinished()) {
		LOG(INFO) << "CGI worker finished"
				  << attr("clientFd", worker->getClientFd())
				  << attr("pid", worker->getPid())
				  << attr("state", worker->getState());
		changes.fdsToRemove.push_back(worker->getReadFd());
		// 書き込みFDがまだ監視対象ならそれも削除リストに追加
		if (_pipeFdToWorker.count(worker->getWriteFd())) {
			changes.fdsToRemove.push_back(worker->getWriteFd());
		}
		// クライアントFDを通知リストに追加
		changes.clientFdsToNotify.push_back(worker->getClientFd());
	}
	return changes;
}

FdEventChanges CgiManager::cleanupTimedOutWorkers() {
	FdEventChanges changes;
	time_t now = time(NULL);
	std::vector< CgiWorker * > workersToCleanup;

	for (std::vector< CgiWorker * >::iterator it = _workers.begin();
		 it != _workers.end(); ++it) {
		time_t elapsed = now - (*it)->getLastActivityTime();
		if (elapsed > _timeoutSeconds) {
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
		changes.fdsToRemove.push_back(worker->getReadFd());
		if (_pipeFdToWorker.count(worker->getWriteFd())) {
			changes.fdsToRemove.push_back(worker->getWriteFd());
		}

		// クライアントFDを通知リストに追加（レスポンス送信のため）
		changes.clientFdsToNotify.push_back(worker->getClientFd());
	}
	return changes;
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
		res.statusCode = HttpStatus::GATEWAY_TIMEOUT;
	} else { // CGI_ERROR
		res.statusCode = HttpStatus::INTERNAL_SERVER_ERROR;
	}

	_removeWorker(worker);
	return true;
}

bool CgiManager::isCgiFd(int fd) const { return _pipeFdToWorker.count(fd) > 0; }
