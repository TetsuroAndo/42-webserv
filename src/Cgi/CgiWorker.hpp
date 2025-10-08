#pragma once

#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/time.h>
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Config/Config.hpp"
#include "ParseCgiResponse.hpp"

class CgiWorker {
public:
	enum CgiState {
		CGI_INIT,
		CGI_SENDING_BODY,
		CGI_RECEIVING_HEADERS,
		CGI_RECEIVING_BODY,
		CGI_COMPLETE,
		CGI_ERROR,
		CGI_TIMEOUT
	};

	CgiWorker(const HttpRequest &req, const Location &locConf, const std::string &scriptPath, const std::string &interpreterPath);
	~CgiWorker();

	void execute();

	void handleWrite();
	void handleRead();

	int getReadFd() const;
	int getWriteFd() const;
	pid_t getPid() const;
	CgiState getState() const;

	void setState(CgiState newState);

	bool isTimeout() const;
	bool isFinished() const;
	
	void createHttpResponse(HttpResponse &res);

private:
	pid_t              _pid;
	int                _pipe_in[2];  // Server -> CGI
	int                _pipe_out[2]; // CGI -> Server
	CgiState           _state;

	std::string        _request_body;
	size_t             _bytes_sent;
	std::string        _response_buffer;

	std::string        _interpreter_path;
	std::string        _script_path;
	std::vector<char*> _envp;
	std::vector<std::string> _envp_strs;

	time_t              _last_activity_time;
	static const int    TIMEOUT_SECONDS; // TODO: タイムアウトオブジェクトに入れ替える

	void _closePipe(int &fd);
	void _updateLastActivityTime();
	void _childProcess();
	void _processResponseBuffer();

	CgiWorker(const CgiWorker&);
	CgiWorker &operator=(const CgiWorker&);
};
