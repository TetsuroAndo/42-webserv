#include "CgiWorker.hpp"
#include "../Server/Client.hpp"
#include "CgiEnvBuilder.hpp"
#include <algorithm>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

CgiWorker::CgiWorker(PipelineContext &ctx, const std::string &scriptPath,
					 const std::string &interpreterPath)
	: _ctx(ctx), _state(CGI_INIT), _clientFd(ctx.ownerClient.getFd()), _pid(-1),
	  _requestBody(ctx.req.getBody()), _bytesSent(0), _responseBuffer(),
	  _scriptPath(scriptPath), _interpreterPath(interpreterPath),
	  _lastActivityTime(time(NULL)) {
	_pipeIn[0] = -1;
	_pipeIn[1] = -1;
	_pipeOut[0] = -1;
	_pipeOut[1] = -1;
}

CgiWorker::~CgiWorker() {
	_closePipe(_pipeIn[1]);
	_closePipe(_pipeOut[0]);

	if (_pid > 0) {
		kill(_pid, SIGKILL);
		int status;
		waitpid(_pid, &status, 0);
	}
}

void CgiWorker::execute() {
	if (pipe(_pipeIn) < 0 || pipe(_pipeOut) < 0) {
		_state = CGI_ERROR;
		return;
	}

	_pid = fork();
	if (_pid < 0) {
		_state = CGI_ERROR;
		_closePipe(_pipeIn[0]);
		_closePipe(_pipeIn[1]);
		_closePipe(_pipeOut[0]);
		_closePipe(_pipeOut[1]);
		return;
	}

	if (_pid == 0) {
		std::vector< std::string > envpStrs =
			CgiEnvBuilder::build(_ctx, _scriptPath);
		_childProcess(_scriptPath, _interpreterPath, envpStrs);
	}

	_closePipe(_pipeIn[0]);
	_closePipe(_pipeOut[1]);

	if (fcntl(_pipeIn[1], F_SETFL, O_NONBLOCK) < 0 ||
		fcntl(_pipeOut[0], F_SETFL, O_NONBLOCK) < 0) {
		_state = CGI_ERROR;
		kill(_pid, SIGKILL);
		_closePipe(_pipeIn[1]);
		_closePipe(_pipeOut[0]);
		return;
	}

	_state = _requestBody.empty() ? CGI_RECEIVING : CGI_SENDING_BODY;
	updateLastActivityTime();
}

void CgiWorker::handleWrite() {
	if (_requestBody.empty()) {
		_closePipe(_pipeIn[1]);
		_state = CGI_RECEIVING;
		return;
	}

	ssize_t bytesToWrite =
		std::min((size_t)4096, _requestBody.size() - _bytesSent);
	ssize_t bytes =
		write(getWriteFd(), _requestBody.c_str() + _bytesSent, bytesToWrite);

	if (bytes < 0) {
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
	char buffer[4096];
	ssize_t bytes = read(getReadFd(), buffer, sizeof(buffer));

	if (bytes < 0) {
		_state = CGI_ERROR;
		_closePipe(_pipeOut[0]);
		return;
	}

	if (bytes == 0) {
		_closePipe(_pipeOut[0]);
		_state = CGI_COMPLETE;
		_responseParser.parse(_responseBuffer);
	} else {
		_responseBuffer.append(buffer, bytes);
	}
	updateLastActivityTime();
}

void CgiWorker::_childProcess(const std::string &scriptPath,
							  const std::string &interpreterPath,
							  const std::vector< std::string > &envp_strs) {
	close(_pipeIn[1]);
	close(_pipeOut[0]);

	if (dup2(_pipeIn[0], STDIN_FILENO) < 0 ||
		dup2(_pipeOut[1], STDOUT_FILENO) < 0) {
		perror("dup2 failed");
		exit(EXIT_FAILURE);
	}

	close(_pipeIn[0]);
	close(_pipeOut[1]);

	std::vector< char * > envp;
	for (size_t i = 0; i < envp_strs.size(); ++i) {
		envp.push_back(const_cast< char * >(envp_strs[i].c_str()));
	}
	envp.push_back(NULL);

	char *const argv[] = {const_cast< char * >(interpreterPath.c_str()),
						  const_cast< char * >(scriptPath.c_str()), NULL};

	execve(interpreterPath.c_str(), argv, envp.data());
	perror("execve failed");
	exit(EXIT_FAILURE);
}

void CgiWorker::_closePipe(int &fd) {
	if (fd != -1) {
		close(fd);
		fd = -1;
	}
}

int CgiWorker::getClientFd() const { return _clientFd; }
int CgiWorker::getReadFd() const { return _pipeOut[0]; }
int CgiWorker::getWriteFd() const { return _pipeIn[1]; }
pid_t CgiWorker::getPid() const { return _pid; }
CgiWorker::CgiState CgiWorker::getState() const { return _state; }
void CgiWorker::setTimeout() { _state = CGI_TIMEOUT; }
time_t CgiWorker::getLastActivityTime() const { return _lastActivityTime; }
void CgiWorker::updateLastActivityTime() { _lastActivityTime = time(NULL); }
bool CgiWorker::isTimeout() const { return _state == CGI_TIMEOUT; }
bool CgiWorker::isFinished() const {
	return _state == CGI_COMPLETE || _state == CGI_ERROR ||
		   _state == CGI_TIMEOUT;
}
void CgiWorker::createHttpResponse(HttpResponse &res) {
	_responseParser.setResponse(res);
}
