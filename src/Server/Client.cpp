#include "Client.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include "HttpConnection.hpp"
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
	: _fd(fd), _listenPort(listenPort), _socket(NULL), _context(NULL),
	  _httpConnection(NULL), _server(server) {
	std::stringstream ipStream;
	const uint32_t ip_addr = ntohl(addr.sin_addr.s_addr);
	_ip = ipToString(ip_addr);
	_port = ntohs(addr.sin_port);

	try {
		_socket = new Socket(config, fd, addr);
		_context = new PipelineContext(config, *this, cgiManager);
		_httpConnection = new HttpConnection(this, _context, this->_server);
	} catch (...) {
		delete _socket;
		delete _context;
		delete _httpConnection;
		_socket = NULL;
		_context = NULL;
		_httpConnection = NULL;
		throw;
	}
}

Client::~Client() {
	delete _httpConnection;
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

	time_t timeoutSec = _httpConnection->calculateTimeout();
	_server->getTimeoutManager().add(this, timeoutSec);
}

void Client::handleReadEvent() { _httpConnection->handleReadEvent(); }

void Client::handleWriteEvent() { _httpConnection->handleWriteEvent(); }

int Client::getFd() const { return _fd; }
Socket *Client::getSocket() const { return _socket; }
PipelineContext *Client::getContext() const { return _context; }
HttpConnection *Client::getHttpConnection() const { return _httpConnection; }
Server *Client::getServer() const { return _server; }

// HttpConnectionEventHandlerの実装
void Client::onConnectionClose(int fd) {
	if (_server) {
		_server->closeConnection(fd);
	}
}

void Client::onSocketModify(int fd, uint32_t events) {
	if (_server) {
		_server->getSocketsManager().modifySocket(fd, events);
	}
}

void Client::onCgiChanges() {
	if (_server) {
		_server->applyCgiChanges();
	}
}

void Client::onRequestProcessed() {
	// リクエスト処理完了時のタイムアウト更新
	updateTimeout();
}

const std::string &Client::getIp() const { return _ip; }
int Client::getPort() const { return _port; }
int Client::getListenPort() const { return _listenPort; }
