#include "CgiManager.hpp"
#include "../Config/Config.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Http/Core/HttpStatus.hpp"
#include "../Http/Resolver/RequestResolver.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "../Server/Client/Client.hpp"
#include "../Server/Client/Events/CgiEndEvent.hpp"
#include "../Server/Client/Events/CgiErrorEvent.hpp"
#include "../Server/Client/Events/CgiErrorExitEvent.hpp"
#include "../Server/Client/Events/CgiReadEvent.hpp"
#include "../Server/Client/Events/CgiWriteEvent.hpp"
#include "../Server/Server.hpp"
#include "../Socket/SocketsManager.hpp"
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

	_clientFdToWorker.erase(worker->getClientFd());
	if (worker->getPid() > 0) {
		_pidToWorker.erase(worker->getPid());
	}

	delete worker;
}

CgiManager::CgiManager(const size_t maxWorkers) : _maxWorkers(maxWorkers) {}

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
	CgiWorker *worker = NULL;
	try {
		// worker上限に達していないかを確認
		if (_workers.size() >= _maxWorkers) {
			LOG(WARNING) << "CGI worker limit reached"
						 << attr("limit", _maxWorkers);
			ctx.res.setStatusCode(HttpStatus::SERVICE_UNAVAILABLE);
			return;
		}
		const Location &loc = ctx.conf->getLocation(ctx.req.getPath());

		// リクエストからスクリプト仮想パスとPATH_INFOを切り出す
		std::string scriptVirtual;
		std::string pathInfo;

		// 有効なCgiスクリプトではなかった場合
		if (!RequestResolver::extractCgiScript(ctx.req.getPath(), loc,
											   scriptVirtual, pathInfo)) {
			LOG(WARNING) << "Failed to extract CGI script from request"
						 << attr("path", ctx.req.getPath());
			ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
			return;
		}

		// スクリプトの実ファイル（絶対パス）を解決
		const std::string scriptPath =
			RequestResolver::resolvePath(scriptVirtual, *ctx.conf);

		// ファイルが存在しなかった場合
		if (scriptPath.empty()) {
			LOG(WARNING) << "CGI script not found"
						 << attr("scriptVirtual", scriptVirtual);
			ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
			return;
		}

		// インタプリタの解決
		std::string interpreterPath;
		const size_t dotPosVirtual = scriptVirtual.rfind('.');
		if (dotPosVirtual != std::string::npos) {
			const std::string ext = scriptVirtual.substr(dotPosVirtual);
			if (loc.cgiConf.count(ext)) {
				interpreterPath = loc.cgiConf.at(ext);
			}
		}

		// Configで設定されているインタプリンタが存在しない場合
		if (interpreterPath.empty()) {
			LOG(WARNING) << "No CGI interpreter found for the request path: "
						 << scriptPath;
			ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
			return;
		}

		// interpreterPathをファイルまでの部分に切り出す
		std::string resolvedInterpreterPath = interpreterPath;
		if (!interpreterPath.empty() && interpreterPath[0] != '/') {
			const size_t lastSlashPos = scriptPath.find_last_of('/');
			if (lastSlashPos != std::string::npos) {
				const std::string scriptDir =
					scriptPath.substr(0, lastSlashPos);
				resolvedInterpreterPath = scriptDir + "/" + interpreterPath;
			}
		}
		// 読み込み権限を確認
		if (access(resolvedInterpreterPath.c_str(), F_OK) != 0) {
			LOG(WARNING) << "CGI interpreter not found"
						 << attr("interpreter", resolvedInterpreterPath);
			ctx.res.setStatusCode(HttpStatus::NOT_FOUND);
			return;
		}
		// 実装権限を確認
		if (access(resolvedInterpreterPath.c_str(), X_OK) != 0) {
			LOG(WARNING) << "CGI interpreter not executable"
						 << attr("interpreter", resolvedInterpreterPath);
			ctx.res.setStatusCode(HttpStatus::FORBIDDEN);
			return;
		}

		LOG(DEBUG) << "Using CGI interpreter"
				   << attr("interpreterPath", interpreterPath)
				   << attr("resolvedInterpreterPath", resolvedInterpreterPath)
				   << attr("script", scriptPath);

		// workerの作成・実行
		worker = new CgiWorker(ctx, scriptPath, interpreterPath, this);

		_workers.push_back(worker);

		// workerの作成の成否を確認
		if (worker->getState() == CgiWorker::CGI_ERROR) {
			ctx.res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
			return;
		}

		// workerとFDを紐付けする
		_clientFdToWorker[worker->getClientFd()] = worker;
		_pidToWorker[worker->getPid()] = worker;

		// EventManagerに監視対象のFDを追加
		EventManager &eventManager = ctx.ownerClient.getEventManager();
		const SocketsManager &socketsManager =
			ctx.ownerClient.getServer().getSocketsManager();
		int writeEventFd = -1;
		{
			LOG(DEBUG) << "set cgi read fd" << attr("fd", worker->getReadFd());
			const int fd = worker->getReadFd();
			socketsManager.registerSocket(fd, EPOLLIN);
			eventManager.initFd(fd);
			eventManager.addEvent(fd, new CgiReadEvent(worker));
		}
		{
			const int fd = worker->getWriteFd();
			if (fd != -1 && (ctx.req.getMethod() == "POST" ||
							 ctx.req.getMethod() == "PUT")) {
				socketsManager.registerSocket(fd, EPOLLOUT);
				socketsManager.modifySocket(fd, EPOLLOUT);
				eventManager.initFd(fd);
				eventManager.addEvent(fd, new CgiWriteEvent(worker));
				writeEventFd = fd;
			}
		}
		{
			const int fd = worker->getErrFd();
			socketsManager.registerSocket(fd, EPOLLIN);
			eventManager.initFd(fd);
			eventManager.addEvent(fd, new CgiErrorEvent(worker));
		}
		{
			const int fd = worker->getCompletionFdOut();
			socketsManager.registerSocket(fd, EPOLLIN);
			eventManager.initFd(fd);
			eventManager.addEvent(fd,
								  new CgiEndEvent(&ctx.ownerClient, worker));
		}
		{
			const int fd = worker->getStatusFd();
			socketsManager.registerSocket(fd, EPOLLIN);
			eventManager.initFd(fd);
			eventManager.addEvent(fd, new CgiErrorExitEvent(worker));
		}

		worker->setEventFds(worker->getReadFd(), writeEventFd,
							worker->getErrFd(), worker->getStatusFd(),
							worker->getCompletionFdOut());

		LOG(INFO) << "CGI worker created"
				  << attr("clientFd", worker->getClientFd())
				  << attr("pid", worker->getPid())
				  << attr("readFd", worker->getReadFd())
				  << attr("writeFd", worker->getWriteFd())
				  << attr("errFd", worker->getErrFd());
		if (worker->isFinished()) {
			_completedClients.push(worker->getClientFd());
		}

	} catch (const std::exception &e) {
		// new または execute で失敗した場合に備えて delete (NULLでも問題なし)
		delete worker;
		LOG(ERROR) << "Failed to create CGI worker: " << e.what();
		ctx.res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
	}
}

void CgiManager::cleanupTimedOutWorkers() {
	const time_t now = std::time(NULL);
	std::vector< CgiWorker * > workersToCleanup;

	for (std::vector< CgiWorker * >::iterator it = _workers.begin();
		 it != _workers.end(); ++it) {
		if (now - (*it)->getLastActivityTime() >
			(*it)->getTimeoutSeconds()) {
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
			LOG(DEBUG) << "CGI process reaped by cleanupFinishedWorkers"
					   << attr("pid", pid) << attr("status", status);

			_pidToWorker.erase(pid);
			worker->setExitStatus(status);
			if (!worker->isTimeout()) {
				if (WIFSIGNALED(status)) {
					LOG(WARNING)
						<< "CGI process terminated by signal"
						<< attr("pid", pid) << attr("signal", WTERMSIG(status));
					worker->setError();
				} else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
					LOG(WARNING) << "CGI process exited with non-zero status"
								 << attr("pid", pid)
								 << attr("status", WEXITSTATUS(status));
					if (!worker->hasStatusHeader()) {
						worker->setError();
					}
				}
			}
			if (worker->isFinished()) {
				const char tmpC = 'c';
				const int tmp =
					write(worker->getCompletionFdIn(), &tmpC, sizeof(tmpC));
				(void)tmp;
			}
		} else {
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

bool CgiManager::isCgiComplete(const int clientFd, HttpResponse &res) {
	const std::map< int, CgiWorker * >::iterator it =
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
		// Statusヘッダーが設定されている場合は、そのステータスコードを優先する
		if (worker->hasStatusHeader()) {
			worker->createHttpResponse(res);
		} else {
			res.setStatusCode(HttpStatus::INTERNAL_SERVER_ERROR);
		}
	}

	worker->setKeepCompletionEventOnDetach(true);
	_removeWorker(worker);
	return true;
}

void CgiManager::abortClient(const int clientFd) {
	std::map< int, CgiWorker * >::iterator it =
		_clientFdToWorker.find(clientFd);
	if (it == _clientFdToWorker.end())
		return;

	CgiWorker *worker = it->second;

	// プロセスを確実に終了
	pid_t pid = worker->getPid();
	if (pid > 0) {
		kill(pid, SIGKILL);
		waitpid(pid, NULL, WNOHANG);
		_pidToWorker.erase(pid);
	}
	_removeWorker(worker);
}
