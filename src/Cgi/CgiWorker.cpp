#include "CgiWorker.hpp"
#include <stdexcept>
#include <sys/wait.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstring> // for std::strcpy
#include <sstream>
#include <iostream>

// C++98 standard: Define and initialize static const member in the .cpp file
const int CgiWorker::TIMEOUT_SECONDS = 30;

CgiWorker::CgiWorker(const HttpRequest &req, const Location &locConf,
					 const std::string &scriptPath, const std::string &interpreterPath)
	: _pid(-1), _state(CGI_START), _request_body(req.getBody()),
	  _bytes_sent(0), _interpreter_path(interpreterPath),
	  _script_path(scriptPath)
{
	_pipe_in[0] = -1; _pipe_in[1] = -1;
	_pipe_out[0] = -1; _pipe_out[1] = -1;
	_setupEnvironment(req, locConf);
	_updateLastActivityTime();
}

CgiWorker::~CgiWorker() {
	_closePipe(_pipe_in[1]);
	_closePipe(_pipe_out[0]);
	
	if (_pid > 0) {
		// kill and waitpid are now managed by CgiManager
	}
}

void CgiWorker::execute() {
	if (pipe(_pipe_in) < 0 || pipe(_pipe_out) < 0) {
		throw std::runtime_error("pipe() failed");
	}

	_pid = fork();
	if (_pid < 0) {
		_closePipe(_pipe_in[0]); _closePipe(_pipe_in[1]);
		_closePipe(_pipe_out[0]); _closePipe(_pipe_out[1]);
		throw std::runtime_error("fork() failed");
	}

	if (_pid == 0) {
		_childProcess();
	}

	// Parent process
	close(_pipe_in[0]);  _pipe_in[0] = -1;
	close(_pipe_out[1]); _pipe_out[1] = -1;

	// Set pipes to non-blocking
	if (fcntl(_pipe_in[1], F_SETFL, O_NONBLOCK) == -1 || fcntl(_pipe_out[0], F_SETFL, O_NONBLOCK) == -1) {
		throw std::runtime_error("fcntl() failed");
	}
	
	_state = _request_body.empty() ? CGI_RECEIVING : CGI_SENDING_BODY;
}

void CgiWorker::_childProcess() {
	close(_pipe_in[1]);
	close(_pipe_out[0]);
	
	if (dup2(_pipe_in[0], STDIN_FILENO) == -1 || dup2(_pipe_out[1], STDOUT_FILENO) == -1) {
		std::cerr << "dup2() failed in child process" << std::endl;
		exit(EXIT_FAILURE);
	}
	close(_pipe_in[0]);
	close(_pipe_out[1]);

	// Create envp (char**) from _envp_strs (vector<string>)
	std::vector<char*> envp;
	for (size_t i = 0; i < _envp_strs.size(); ++i) {
		envp.push_back(const_cast<char*>(_envp_strs[i].c_str()));
	}
	envp.push_back(NULL);

	char* const argv[] = {const_cast<char*>(_interpreter_path.c_str()), const_cast<char*>(_script_path.c_str()), NULL};
	
	// Change directory
	std::string script_dir = _script_path.substr(0, _script_path.find_last_of("/"));
	if (chdir(script_dir.c_str()) != 0) {
		std::cerr << "chdir() to " << script_dir << " failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	execve(_interpreter_path.c_str(), argv, &envp[0]);

	// If execve returns, it's an error
	std::cerr << "execve() failed for " << _interpreter_path << std::endl;
	exit(EXIT_FAILURE);
}


void CgiWorker::handleWrite() {
	if (_state != CGI_SENDING_BODY) return;
	_updateLastActivityTime();
	
	ssize_t bytes = write(_pipe_in[1], _request_body.c_str() + _bytes_sent, _request_body.length() - _bytes_sent);
	
	if (bytes > 0) {
		_bytes_sent += bytes;
	}

	if (_bytes_sent >= _request_body.length()) {
		_closePipe(_pipe_in[1]);
		_state = CGI_RECEIVING;
	} else if (bytes < 0) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			_state = CGI_ERROR;
			_closePipe(_pipe_in[1]);
		}
	}
}

void CgiWorker::handleRead() {
	if (_state != CGI_RECEIVING) return;
	_updateLastActivityTime();

	char buffer[4096];
	ssize_t bytes_read = read(_pipe_out[0], buffer, sizeof(buffer));

	if (bytes_read > 0) {
		_response_buffer.append(buffer, bytes_read);
	} else if (bytes_read == 0) { // EOF
		_state = CGI_COMPLETE;
		_closePipe(_pipe_out[0]);
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			_state = CGI_ERROR;
			_closePipe(_pipe_out[0]);
		}
	}
}

void CgiWorker::createHttpResponse(HttpResponse &res) {
	if (_state == CGI_COMPLETE) {
		CgiResponseParser parser;
		parser.parse(_response_buffer);
		parser.setResponse(res);
	} else if (_state == CGI_ERROR) {
		res.setStatusCode(500);
		// ... set error response ...
	} else if (_state == CGI_TIMEOUT) {
		// SIGKILL is sent from manager, but let's be safe
		int status;
		if (waitpid(_pid, &status, WNOHANG) == 0) {
			kill(_pid, SIGKILL);
			waitpid(_pid, NULL, 0);
		}
		res.setStatusCode(504);
		// ... set timeout response ...
	}
}

// ... (rest of the methods: _setupEnvironment, _closePipe, etc.) ...
// No major changes needed for _setupEnvironment but simplified envp memory management is reflected above.

void CgiWorker::_setupEnvironment(const HttpRequest &req, const Location &locConf) {
	(void)locConf;
	std::map<std::string, std::string> envMap;
	// ... (Same as before) ...
	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
	envMap["REQUEST_METHOD"] = req.getMethod();
	envMap["SCRIPT_FILENAME"] = _script_path;
	envMap["SCRIPT_NAME"] = req.getPath();
	envMap["QUERY_STRING"] = req.getQuery();
	envMap["CONTENT_TYPE"] = req.getHeader("Content-Type");
	std::stringstream ss;
	ss << req.getBody().length();
	envMap["CONTENT_LENGTH"] = ss.str();
	const std::map<std::string, std::string> &headers = req.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::string envKey = "HTTP_" + it->first;
		for (size_t i = 0; i < envKey.length(); ++i) {
			if (envKey[i] == '-') envKey[i] = '_';
			else envKey[i] = toupper(envKey[i]);
		}
		envMap[envKey] = it->second;
	}

	_envp_strs.reserve(envMap.size());
	for (std::map<std::string, std::string>::iterator it = envMap.begin(); it != envMap.end(); ++it) {
		_envp_strs.push_back(it->first + "=" + it->second);
	}
}

void CgiWorker::_closePipe(int &fd) {
	if (fd != -1) {
		close(fd);
		fd = -1;
	}
}

void CgiWorker::_updateLastActivityTime() { _last_activity_time = time(NULL); }
bool CgiWorker::isTimeout() const { return (time(NULL) - _last_activity_time) > TIMEOUT_SECONDS; }
bool CgiWorker::isFinished() const { return _state == CGI_COMPLETE || _state == CGI_ERROR || _state == CGI_TIMEOUT; }
int CgiWorker::getReadFd() const { return _pipe_out[0]; }
int CgiWorker::getWriteFd() const { return _pipe_in[1]; }
pid_t CgiWorker::getPid() const { return _pid; }
CgiState CgiWorker::getState() const { return _state; }
void CgiWorker::setState(CgiState newState) { _state = newState; }
