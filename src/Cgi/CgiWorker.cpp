#include "CgiWorker.hpp"
#include "Client.hpp"
#include <algorithm>
#include <unistd.h>

CgiWorker::CgiWorker(PipelineContext &ctx, const std::string &scriptPath,
					 const std::string &interpreterPath)
	: _state(CGI_INIT), _clientFd(ctx.ownerClient.getFd()), _pid(-1),
	  _pipeIn{-1, -1}, _pipeOut{-1, -1}, _requestBody(ctx.recvBuffer),
	  _bytesSent(0), _responseBuffer(ctx.recvBuffer), _scriptPath(scriptPath),
	  _interpreterPath(interpreterPath), _responseParser(),
	  _lastActivityTime(time(NULL)) {}

CgiWorker::~CgiWorker() { // TODO: プロセス終了の待機
}

void CgiWorker::execute() {}

void CgiWorker::handleWrite() {}

void CgiWorker::handleRead() {}

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
