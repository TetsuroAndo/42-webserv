#include "Client.hpp"
#include "../../Handler/ErrorHandler.hpp"
#include "../../Http/Builder/ResponseBuilder.hpp"
#include "../../Http/Core/HttpRequest.hpp"
#include "../../Http/Core/HttpResponse.hpp"
#include "../../Http/Core/HttpStatus.hpp"
#include "../../Lib/Logger/Log.hpp"
#include "../../Lib/StringOps/StringOps.hpp"
#include "../../Middleware/Core/PipelineContext.hpp"
#include "../Server.hpp"
#include "Events/ReadEvent.hpp"
#include "Events/WriteEvent.hpp"
#include "HttpConnection.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sstream>
#include <unistd.h>

// clang-format off
Client::Client(const int fd, const sockaddr_in &addr, const int listenPort,
			Server &server, EventManager &eventManager)
	: _server(server),
	  _fd(fd),
	  _listenPort(listenPort),
	  _socket(server.getConfig(), fd, addr),
	  _context(server.getConfig(), *this, server.getCgiManager()),
	  _httpConnection(this, _context, *this),
	  _eventManager(eventManager)
{
	const unsigned int ip_addr = ntohl(addr.sin_addr.s_addr);
	_ip = StringOps::ipToString(ip_addr);
	_port = ntohs(addr.sin_port);
}
// clang-format on

Client::~Client() {}

Server &Client::getServer() const { return _server; }

int Client::getFd() const { return _fd; }
int Client::getPort() const { return _port; }
int Client::getListenPort() const { return _listenPort; }
const std::string &Client::getIp() const { return _ip; }

Socket &Client::getSocket() { return _socket; }
const Socket &Client::getSocket() const { return _socket; }
PipelineContext &Client::getContext() { return _context; }
const PipelineContext &Client::getContext() const { return _context; }

HttpConnection &Client::getHttpConnection() { return _httpConnection; }
const HttpConnection &Client::getHttpConnection() const {
	return _httpConnection;
}

void Client::onTimeout() {
	LOG(INFO) << "Client timed out for fd: " << _fd;
	_context.res.setStatusCode(HttpStatus::REQUEST_TIMEOUT);
	ErrorHandler handler;
	handler.handle(_context);
	const std::string response = ResponseBuilder::build(_context.res);
	getSocket().setSendBuffer(response);
	getHttpConnection().handleWriteEvent();
	_eventManager.removeFd(this->getFd());
}

void Client::handleReadEvent() { _httpConnection.handleReadEvent(); }
void Client::handleWriteEvent() { _httpConnection.handleWriteEvent(); }
void Client::updateTimeout() {
	const time_t timeoutSec = _httpConnection.calculateTimeout();
	_server.getTimeoutManager().add(this, timeoutSec);
}

// HttpConnectionEventHandlerの実装
void Client::onConnectionClose(int fd) { _server.closeConnection(fd); }

void Client::onSocketModify(int fd, uint32_t events) {
	_server.getSocketsManager().modifySocket(fd, events);
}

void Client::onCgiChanges() { _server.applyCgiChanges(); }

void Client::onRequestProcessed() {
	// リクエスト処理完了時のタイムアウト更新
	updateTimeout();
}
