#include "CgiManager.hpp"
#include <algorithm>

void CgiManager::_removeWorker(CgiWorker *worker) {
	_workers.erase(std::remove(_workers.begin(), _workers.end(), worker),
				   _workers.end());
	_pipeFdToWorker.erase(worker->getReadFd());
	_clientFdToWorker.erase(worker->getClientFd());
	delete worker;
}

CgiManager::CgiManager(const Config &config)
	: _workers(), _pipeFdToWorker(), _clientFdToWorker(),
	  _timeoutSeconds(config.getTimeoutSec()) {}

CgiManager::~CgiManager() {
	while (!_workers.empty()) {
		_removeWorker(_workers.back());
	}
}
