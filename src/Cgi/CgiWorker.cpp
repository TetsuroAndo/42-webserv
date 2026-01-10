#include "CgiWorker.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Server/Client/Client.hpp"
#include "../Server/Server.hpp"
#include "CgiEnvBuilder.hpp"
#include "CgiManager.hpp"
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
void blockForeverNoCpu() {
	// ReSharper disable once CppDFAEndlessLoop
	while (true) {
		;
	}
}

void notifyErrorToParentAndStop(const int statusWriteFd, const int err) {
	const ssize_t ret = write(statusWriteFd, &err, sizeof(err));
	if (ret < 0) {
		LOG(ERROR) << "write failed: " << strerror(errno);
	}
	blockForeverNoCpu();
}
} // namespace

CgiWorker::CgiWorker(PipelineContext &ctx, const std::string &scriptPath,
					 const std::string &interpreterPath, CgiManager *manager)
	: _ctx(ctx), _manager(manager), _state(CGI_INIT),
	  _clientFd(ctx.ownerClient.getFd()), _pid(-1), _exitStatus(-1),
	  _exitStatusSet(false), _outputComplete(false), _completionNotified(false),
	  _requestBody(ctx.req.getBody()), _bytesSent(0), _scriptPath(scriptPath),
	  _interpreterPath(interpreterPath), _lastActivityTime(std::time(NULL)),
	  _timeoutSeconds(static_cast< time_t >(ctx.conf->getTimeoutSec())),
	  _readBuffer(ctx.conf->getPerformance().cgiIoBufferSize),
	  _errBuffer(ctx.conf->getPerformance().cgiIoBufferSize) {
	_eventReadFd = -1;
	_eventWriteFd = -1;
	_eventErrFd = -1;
	_eventStatusFd = -1;
	_eventCompletionFd = -1;
	_keepCompletionEventOnDetach = false;
	_pipeIn[0] = -1;
	_pipeIn[1] = -1;
	_pipeOut[0] = -1;
	_pipeOut[1] = -1;
	_pipeErr[0] = -1;
	_pipeErr[1] = -1;
	_pipeStatus[0] = -1;
	_pipeStatus[1] = -1;
	_pipeComplete[0] = -1;
	_pipeComplete[1] = -1;

	execute();
}

CgiWorker::~CgiWorker() {
	detachEvents(_keepCompletionEventOnDetach);
	_closePipe(_pipeIn[1]);
	_closePipe(_pipeOut[0]);
	_closePipe(_pipeErr[0]);
	if (!_keepCompletionEventOnDetach) {
		_closePipe(_pipeComplete[0]);
	}
	_closePipe(_pipeComplete[1]);
	_closePipe(_pipeStatus[0]);

	if (0 < _pid) {
		// プロセスがまだ終了していないか確認（非ブロッキング）
		int status;
		const pid_t result = waitpid(_pid, &status, WNOHANG);

		if (result == 0) {
			LOG(DEBUG)
				<< "CgiWorker destroyed, sending SIGKILL to running child"
				<< attr("pid", _pid);
			kill(_pid, SIGKILL);
			// ここで waitpid(..., 0) を呼んで待つ必要はない。
			// OS (init) がそのうち回収する (ゾンビになるが許容する)
		}
		// result > 0 (既に終了) または result == -1 (ECHILD)
		// の場合は何もしなくて良い
	}
}

void CgiWorker::detachEvents(const bool keepCompletionEvent) const {
	EventManager &eventManager = _ctx.ownerClient.getEventManager();
	SocketsManager &socketsManager =
		_ctx.ownerClient.getServer().getSocketsManager();
	int fds[5] = {_eventReadFd, _eventWriteFd, _eventErrFd, _eventStatusFd,
				  _eventCompletionFd};
	const size_t fdsCount = sizeof(fds) / sizeof(fds[0]);
	for (size_t i = 0; i < fdsCount; ++i) {
		const int fd = fds[i];
		if (fd < 0) {
			continue;
		}
		if (keepCompletionEvent && fd == _eventCompletionFd) {
			continue;
		}
		socketsManager.unregisterSocket(fd);
		eventManager.forgetFd(fd);
	}
}

void CgiWorker::setKeepCompletionEventOnDetach(const bool keepCompletionEvent) {
	_keepCompletionEventOnDetach = keepCompletionEvent;
}

void CgiWorker::setEventFds(const int readFd, const int writeFd,
							const int errFd, const int statusFd,
							const int completionFd) {
	_eventReadFd = readFd;
	_eventWriteFd = writeFd;
	_eventErrFd = errFd;
	_eventStatusFd = statusFd;
	_eventCompletionFd = completionFd;
}

void CgiWorker::handleErrorExit() {
	int childErr = 0;
	const ssize_t n = read(_pipeStatus[0], &childErr, sizeof(childErr));

	if (0 < n) {
		LOG(ERROR) << "CGI child failed" << attr("pid", _pid)
				   << attr("errno", childErr)
				   << attr("msg", strerror(childErr));

		_state = CGI_ERROR;
		kill(_pid, SIGKILL);
		setExitStatus(childErr);
		_closePipe(_pipeStatus[0]);
		_outputComplete = true;
		_exitStatusSet = true;

		if (isFinished()) {
			const char tmpC = 'x';
			const int tmp = write(_pipeComplete[1], &tmpC, sizeof(tmpC));
			if (tmp <= 0) {
				LOG(ERROR) << "Failed to write to CGI worker: "
						   << strerror(errno);
			}
		}
	}
}

void CgiWorker::execute() {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, _pipeIn) < 0 ||
		socketpair(AF_UNIX, SOCK_STREAM, 0, _pipeOut) < 0 ||
		socketpair(AF_UNIX, SOCK_STREAM, 0, _pipeErr) < 0 ||
		socketpair(AF_UNIX, SOCK_STREAM, 0, _pipeStatus) < 0 ||
		socketpair(AF_UNIX, SOCK_STREAM, 0, _pipeComplete) < 0) {
		_state = CGI_ERROR;
		_outputComplete = true;
		_exitStatusSet = true;
		return;
	}

	_pid = fork();
	if (_pid < 0) {
		_state = CGI_ERROR;
		_closePipe(_pipeIn[0]);
		_closePipe(_pipeIn[1]);
		_closePipe(_pipeOut[0]);
		_closePipe(_pipeOut[1]);
		_closePipe(_pipeErr[0]);
		_closePipe(_pipeErr[1]);
		_closePipe(_pipeStatus[1]);
		_closePipe(_pipeComplete[0]);
		_closePipe(_pipeComplete[1]);
		_outputComplete = true;
		_exitStatusSet = true;
		return;
	}

	// 子プロセス
	if (_pid == 0) {
		close(_pipeIn[1]);
		close(_pipeOut[0]);
		close(_pipeErr[0]);
		close(_pipeStatus[0]);
		close(_pipeComplete[0]);
		close(_pipeComplete[1]);
		fcntl(_pipeStatus[1], F_SETFD, FD_CLOEXEC);
		try {
			const std::vector< std::string > envpStrs =
				CgiEnvBuilder::build(_ctx, _scriptPath);
			if (envpStrs.empty()) {
				notifyErrorToParentAndStop(_pipeStatus[1], EINVAL);
			}
			_childProcess(_scriptPath, _interpreterPath, envpStrs);
		} catch (const std::exception &e) {
			const std::string msg =
				std::string("CGI environment build failed: ") + e.what();
			LOG(ERROR) << msg;
			const int err = errno ? errno : ECHILD;
			notifyErrorToParentAndStop(_pipeStatus[1], err);
		} catch (...) {
			const std::string msg = "Unknown error in CGI child process\n";
			LOG(ERROR) << msg;
			const int err = errno ? errno : ECHILD;
			notifyErrorToParentAndStop(_pipeStatus[1], err);
		}
		return; // 呼ばれないはず
	}

	// 親プロセス

	_closePipe(_pipeIn[0]);
	_closePipe(_pipeOut[1]);
	_closePipe(_pipeErr[1]);
	_closePipe(_pipeStatus[1]);

	fcntl(_pipeStatus[0], F_SETFL, O_NONBLOCK);

	if (fcntl(_pipeIn[1], F_SETFL, O_NONBLOCK) < 0 ||
		fcntl(_pipeOut[0], F_SETFL, O_NONBLOCK) < 0 ||
		fcntl(_pipeErr[0], F_SETFL, O_NONBLOCK) < 0 ||
		fcntl(_pipeComplete[0], F_SETFL, O_NONBLOCK) < 0 ||
		fcntl(_pipeComplete[1], F_SETFL, O_NONBLOCK) < 0) {
		_state = CGI_ERROR;
		kill(_pid, SIGKILL);
		_closePipe(_pipeIn[1]);
		_closePipe(_pipeOut[0]);
		_closePipe(_pipeErr[0]);
		_closePipe(_pipeComplete[0]);
		_closePipe(_pipeComplete[1]);
		_outputComplete = true;
		return;
	}

	if (_requestBody.empty()) {
		_closePipe(_pipeIn[1]);
		_state = CGI_RECEIVING;
	} else {
		_state = CGI_SENDING_BODY;
	}
	updateLastActivityTime();
}

void CgiWorker::handleWrite() {
	if (getWriteFd() < 0) {
		return;
	}

	if (_state == CGI_ERROR || _state == CGI_TIMEOUT) {
		_closePipe(_pipeIn[1]);
		return;
	}

	if (_requestBody.empty()) {
		_closePipe(_pipeIn[1]);
		_state = CGI_RECEIVING;
		return;
	}

	if (_bytesSent == 0) {
		LOG(DEBUG) << "Sending request body to CGI"
				   << attr("clientFd", _clientFd) << attr("pid", _pid)
				   << attr("bodySize", _requestBody.size())
				   << attr("body", _requestBody);
	}

	const ssize_t bytesToWrite =
		std::min(static_cast< size_t >(4096), _requestBody.size() - _bytesSent);
	const ssize_t bytes =
		write(getWriteFd(), _requestBody.c_str() + _bytesSent, bytesToWrite);

	if (bytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			// 次のEPOLLOUTで再試行
			return;
		}
		LOG(ERROR) << "Write error in CGI" << attr("error", strerror(errno));
		_state = CGI_ERROR;
		_closePipe(_pipeIn[1]);
		return;
	}
	_bytesSent += bytes;
	if (_bytesSent >= _requestBody.size()) {
		_closePipe(_pipeIn[1]);
		_state = CGI_RECEIVING;
	}
	updateLastActivityTime();
}

void CgiWorker::handleRead() {
	if (getReadFd() < 0) {
		return;
	}
	const ssize_t bytes =
		read(getReadFd(), &_readBuffer[0], _readBuffer.size());

	if (bytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			// 次のEPOLLINで再試行
			return;
		}
		LOG(ERROR) << "Read error in CGI" << attr("error", strerror(errno));
		_state = CGI_ERROR;
		_closePipe(_pipeOut[0]);
		_outputComplete = true;
		if (_manager) {
			_manager->cleanupFinishedWorkers();
		}
		return;
	}

	if (bytes == 0) {
		_closePipe(_pipeOut[0]);
		_responseParser.parse(_responseBuffer);
		const bool headersFound = _responseParser.headersFound();

		// waitpid() はメインループで一元管理される
		// 子プロセスのステータス回収は CgiManager::cleanupFinishedWorkers()
		// で非ブロッキングに行う

		// ヘッダが見つからない場合のみ CGI_ERROR と判定
		if (!headersFound) {
			LOG(WARNING) << "CGI response has no headers" << attr("pid", _pid)
						 << attr("output", _responseBuffer);
			_state = CGI_ERROR;
		} else if (_state != CGI_ERROR && _state != CGI_TIMEOUT) {
			// CGIレスポンスの受信完了
			_state = CGI_COMPLETE;
		}

		_outputComplete = true;

		if (isFinished()) {
			const char tmpC = 'x';
			const int tmp = write(_pipeComplete[1], &tmpC, sizeof(tmpC));
			if (tmp <= 0) {
				LOG(ERROR) << "Failed to write to CGI worker: "
						   << strerror(errno);
			}
		}
	} else {
		const size_t MAX_CGI_RESPONSE_SIZE = 10 * 1024 * 1024;
		if (MAX_CGI_RESPONSE_SIZE < _responseBuffer.size() + bytes) {
			LOG(ERROR) << "CGI response exceeds maximum size"
					   << attr("current", _responseBuffer.size())
					   << attr("limit", MAX_CGI_RESPONSE_SIZE);
			_state = CGI_ERROR;
			_closePipe(_pipeOut[0]);
			_outputComplete = true;
			if (_manager) {
				_manager->cleanupFinishedWorkers();
			}
			return;
		}
		_responseBuffer.append(&_readBuffer[0], bytes);
	}
	updateLastActivityTime();
}

void CgiWorker::handleReadErr() {
	if (getErrFd() < 0) {
		return;
	}
	const ssize_t bytes = read(getErrFd(), &_errBuffer[0], _errBuffer.size());

	if (bytes < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
			return;
		}
		LOG(ERROR) << "Read error from CGI stderr"
				   << attr("error", strerror(errno));
		_closePipe(_pipeErr[0]);
		return;
	}

	if (bytes == 0) {
		_closePipe(_pipeErr[0]);
	} else {
		std::string errOutput(&_errBuffer[0], bytes);
		LOG(ERROR) << "CGI stderr output" << attr("clientFd", _clientFd)
				   << attr("pid", _pid) << attr("stderr", errOutput);
	}
	updateLastActivityTime();
}

int CgiWorker::getClientFd() const { return _clientFd; }

int CgiWorker::getReadFd() const { return _pipeOut[0]; }

int CgiWorker::getWriteFd() const { return _pipeIn[1]; }

int CgiWorker::getErrFd() const { return _pipeErr[0]; }

int CgiWorker::getStatusFd() const { return _pipeStatus[0]; }

int CgiWorker::getCompletionFdOut() const { return _pipeComplete[0]; }

int CgiWorker::getCompletionFdIn() const { return _pipeComplete[1]; }

pid_t CgiWorker::getPid() const { return _pid; }

CgiWorker::CgiState CgiWorker::getState() const { return _state; }

void CgiWorker::setTimeout() { _state = CGI_TIMEOUT; }

void CgiWorker::setError() { _state = CGI_ERROR; }

time_t CgiWorker::getLastActivityTime() const { return _lastActivityTime; }

time_t CgiWorker::getTimeoutSeconds() const { return _timeoutSeconds; }

void CgiWorker::updateLastActivityTime() { _lastActivityTime = time(NULL); }

bool CgiWorker::isTimeout() const { return _state == CGI_TIMEOUT; }

bool CgiWorker::isFinished() const {
	if (_state == CGI_TIMEOUT) {
		return true;
	}
	return _outputComplete && _exitStatusSet;
}

void CgiWorker::createHttpResponse(HttpResponse &res) {
	_responseParser.setResponse(res);
}

void CgiWorker::setExitStatus(const int status) {
	_exitStatus = status;
	_exitStatusSet = true;
}

int CgiWorker::getExitStatus() const { return _exitStatus; }

bool CgiWorker::hasStatusHeader() const {
	return _responseParser.hasStatusHeader();
}

bool CgiWorker::isExitStatusSet() const { return _exitStatusSet; }

CgiManager *CgiWorker::getManager() const { return _manager; }

bool CgiWorker::isCompletionNotified() const { return _completionNotified; }

void CgiWorker::setCompletionNotified() { _completionNotified = true; }

void CgiWorker::_childProcess(
	const std::string &scriptPath, const std::string &interpreterPath,
	const std::vector< std::string > &envp_strs) const {
	close(_pipeIn[1]);
	close(_pipeOut[0]);
	close(_pipeErr[0]);

	if (dup2(_pipeIn[0], STDIN_FILENO) < 0 ||
		dup2(_pipeOut[1], STDOUT_FILENO) < 0 ||
		dup2(_pipeErr[1], STDERR_FILENO) < 0) {
		perror("dup2 failed");
		notifyErrorToParentAndStop(_pipeStatus[1], errno);
	}

	close(_pipeIn[0]);
	close(_pipeOut[1]);
	close(_pipeErr[1]);

	const size_t lastSlashPos = scriptPath.find_last_of('/');
	if (lastSlashPos != std::string::npos) {
		const std::string scriptDir = scriptPath.substr(0, lastSlashPos);
		if (!scriptDir.empty() && chdir(scriptDir.c_str()) < 0) {
			perror("chdir failed");
			notifyErrorToParentAndStop(_pipeStatus[1], errno);
		}
	}

	std::vector< char * > envp;
	for (size_t i = 0; i < envp_strs.size(); ++i) {
		envp.push_back(const_cast< char * >(envp_strs[i].c_str()));
	}
	envp.push_back(NULL);

	char *const argv[] = {const_cast< char * >(interpreterPath.c_str()),
						  const_cast< char * >(scriptPath.c_str()), NULL};

	execve(interpreterPath.c_str(), argv, envp.data());
	perror("execve failed");
	notifyErrorToParentAndStop(_pipeStatus[1], errno);
}

void CgiWorker::_closePipe(int &fd) {
	if (fd != -1) {
		close(fd);
		fd = -1;
	}
}
