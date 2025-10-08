#include "CgiManager.hpp"
#include <sys/wait.h>
#include <iostream>
#include <sys/epoll.h>

CgiManager::CgiManager() {}

CgiManager::~CgiManager() {
	for (std::vector<CgiWorker*>::iterator it = _workers.begin(); it != _workers.end(); ++it) {
		delete *it;
	}
	_workers.clear();
	_fdToWorker.clear();
	_clientFdToWorker.clear();
}

FdEventChanges CgiManager::createWorker(int clientFd, const HttpRequest& req,
										const Location& locConf,
										const std::string& scriptPath,
										const std::string& interpreterPath) {
	FdEventChanges changes;
	CgiWorker* worker = NULL;
	try {
		worker = new CgiWorker(req, locConf, scriptPath, interpreterPath);
		worker->execute();

		_workers.push_back(worker);
		_fdToWorker[worker->getReadFd()] = worker;
		_fdToWorker[worker->getWriteFd()] = worker;
		_clientFdToWorker[clientFd] = worker;

		changes.fdsToAdd.push_back((FdEvent){worker->getReadFd(), EPOLLIN});
		if (!req.getBody().empty()) {
			changes.fdsToAdd.push_back((FdEvent){worker->getWriteFd(), EPOLLOUT});
		}
	} catch (const std::exception& e) {
		std::cerr << "CGI Worker creation failed: " << e.what() << std::endl;
		if (worker) {
			delete worker;
		}
		// TODO: ここでクライアントに500エラーを返す処理が必要
		// HTTPのBuilderモジュールで簡単に返せるライブラリを実装する
	}
	return changes;
}

FdEventChanges CgiManager::handleEvent(int fd) {
	FdEventChanges changes;
	std::map<int, CgiWorker*>::iterator it = _fdToWorker.find(fd);
	if (it == _fdToWorker.end()) {
		return changes;
	}

	CgiWorker* worker = it->second;
	if (fd == worker->getWriteFd()) {
		worker->handleWrite();
		if (worker->getState() == CgiWorker::CGI_RECEIVING_HEADERS) {
			changes.fdsToRemove.push_back(worker->getWriteFd());
		}
	} else if (fd == worker->getReadFd()) {
		worker->handleRead();
	}
	return changes;
}

FdEventChanges CgiManager::cleanupWorkers() {
	FdEventChanges changes;
	std::vector<CgiWorker*> remainingWorkers;
	for (size_t i = 0; i < _workers.size(); ++i) {
		CgiWorker* worker = _workers[i];
		bool toRemove = false;

		if (worker->isFinished()) {
			toRemove = true;
		}
		else if (worker->isTimeout()) {
			std::cerr << "CGI Worker PID " << worker->getPid() << " timed out." << std::endl;
			worker->setState(CgiWorker::CGI_TIMEOUT);
			toRemove = true;
		}

		if (toRemove) {
			// Zombieプロセスを回収
			int status;
			waitpid(worker->getPid(), &status, WNOHANG);

			// isCgiComplete()で処理されるまでworkerインスタンスは残す
			// ただし、イベント監視対象からは外す
			if (_fdToWorker.count(worker->getReadFd())) {
				changes.fdsToRemove.push_back(worker->getReadFd());
			}
			if (_fdToWorker.count(worker->getWriteFd())) {
				changes.fdsToRemove.push_back(worker->getWriteFd());
			}
		} else {
			remainingWorkers.push_back(worker);
		}
	}
	// isCgiCompleteで処理されなかった完了済みワーカーを削除するロジックが必要
	// 今回は簡単のため、完了したworkerはisCgiCompleteで処理される前提とする
	_workers.swap(remainingWorkers);
}

bool CgiManager::isCgiComplete(int clientFd, HttpResponse& res) {
	std::map<int, CgiWorker*>::iterator it = _clientFdToWorker.find(clientFd);
	if (it == _clientFdToWorker.end()) {
		return false;
	}

	CgiWorker* worker = it->second;
	if (worker->isFinished()) {
		worker->createHttpResponse(res);
		_removeWorker(worker);
		_clientFdToWorker.erase(it);
		return true;
	}
	return false;
}

void CgiManager::_removeWorker(CgiWorker* worker) {
	if (!worker) return;

	if (_fdToWorker.count(worker->getReadFd())) {
		_fdToWorker.erase(worker->getReadFd());
	}
	if (_fdToWorker.count(worker->getWriteFd())) {
		_fdToWorker.erase(worker->getWriteFd());
	}

	for (std::vector<CgiWorker*>::iterator vec_it = _workers.begin(); vec_it != _workers.end(); ++vec_it) {
		if (*vec_it == worker) {
			_workers.erase(vec_it);
			break;
		}
	}
	delete worker;
}
