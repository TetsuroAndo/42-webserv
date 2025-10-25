#include "Client.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <unistd.h>

namespace {
// clang-format off
std::string ipToString(uint32_t ip_addr) {
	std::stringstream ss;
	ss << ((ip_addr >> 24) & 0xFF) << "."
	   << ((ip_addr >> 16) & 0xFF) << "."
	   << ((ip_addr >> 8) & 0xFF) << "."
	   << (ip_addr & 0xFF);
	return ss.str();
}
// clang-format on
} // namespace

Client::Client(const int fd, const sockaddr_in &addr, const int listenPort,
			   CgiManager &cgiManager, const Config &config, Server *server)
	: _fd(fd), _listenPort(listenPort), _server(server) {
	const uint32_t ip_addr = ntohl(addr.sin_addr.s_addr);
	_ip = ipToString(ip_addr);
	_port = ntohs(addr.sin_port);

	_socket = new Socket(config, fd, addr);
	_context = new PipelineContext(config, *this, cgiManager);
}

Client::~Client() {
	delete _socket;
	delete _context;
}

void Client::onTimeout() {
	if (_server) {
		LOG(INFO) << "Client timed out for fd: " << _fd;
		_server->closeConnection(this->getFd());
	}
}

void Client::updateTimeout() {
	if (_server == NULL)
		return;

	RequestParser::ParseState state = _context->parser.getState();
	const Config &conf = _context->conf;
	time_t timeoutSec;

	switch (state) {
	case RequestParser::STATE_REQUEST_LINE:
	case RequestParser::STATE_HEADERS:
		timeoutSec = conf.getRequestHeaderTimeoutSec();
		break;
	case RequestParser::STATE_BODY:
		timeoutSec = conf.getRequestBodyTimeoutSec();
		break;
	case RequestParser::STATE_COMPLETE:
	default:
		timeoutSec = conf.getTimeoutSec();
		break;
	}
	_server->getTimeoutManager().add(this, timeoutSec);
}

void Client::handleReadEvent() {
	char buffer[4096];
	const ssize_t bytesRead = recv(_fd, buffer, sizeof(buffer), 0);

	if (bytesRead > 0) {
		_context->recvBuffer.append(buffer, bytesRead);
	} else if (bytesRead == 0) {
		LOG(INFO) << "Client disconnected gracefully" << attr("fd", _fd);
		_server->closeConnection(_fd);
		return;
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "recv() failed" << attr("fd", _fd)
					   << attr("error", strerror(errno));
			_server->closeConnection(_fd);
		}
		return;
	}

	// 読み込みがあったので現在状態に応じてタイムアウトを更新
	updateTimeout();

	// ミドルウェア処理
	_server->getMainProcessor().handle(*_context);

	// CGI開始チェック
	if (_context->isCgi) {
		_server->applyCgiChanges();
		_context->isCgi = false;
		return;
	}

	if (_context->parser.isComplete() || _context->parser.getErrorCode() != 0) {
		std::string sid = (_context->session ? _context->session->getId() : "");
		AccessLogger::getInstance().log(&_context->req, &_context->res, _ip,
										_port, sid);
		const std::string responseStr = ResponseBuilder::build(_context->res);
		if (!responseStr.empty()) {
			_socket->setSendBuffer(_socket->getSendBuffer() + responseStr);
		}
		if (!_socket->getSendBuffer().empty()) {
			_server->getSocketsManager().modifySocket(_fd, EPOLLIN | EPOLLOUT);
		}
	}
}

void Client::handleWriteEvent() {
	const std::string &sendBuffer = _socket->getSendBuffer();
	if (sendBuffer.empty()) {
		_server->getSocketsManager().modifySocket(_fd, EPOLLIN);
		return;
	}

	const ssize_t bytesSent =
		send(_fd, sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		_socket->eraseSendBuffer(0, bytesSent);
		updateTimeout();
		if (_socket->getSendBuffer().empty()) {
			PipelineContext *ctx = _context;
			if (ctx->res.getHeader("Connection") == "close") {
				_server->closeConnection(_fd);
			} else {
				_server->getSocketsManager().modifySocket(_fd, EPOLLIN);
				ctx->reset(_server->getConfig());
			}
		}
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "send() failed" << attr("fd", _fd)
					   << attr("error", strerror(errno));
			_server->closeConnection(_fd);
		}
	}
}

int Client::getFd() const { return _fd; }
Socket *Client::getSocket() const { return _socket; }
PipelineContext *Client::getContext() const { return _context; }
const std::string &Client::getIp() const { return _ip; }
int Client::getPort() const { return _port; }
int Client::getListenPort() const { return _listenPort; }
