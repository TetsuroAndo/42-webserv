#include "HttpConnection.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

HttpConnection::HttpConnection(Client *client, PipelineContext &context,
							   HttpConnectionEventHandler &eventHandler)
	: _client(client), _context(context), _eventHandler(eventHandler),
	  _readBuffer(context.conf.getPerformance().ioBuffersSize) {}

HttpConnection::~HttpConnection() {}

void HttpConnection::processRequest() {
	handleRequest();
	generateResponse();
}

void HttpConnection::handleReadEvent() {
	const ssize_t bytesRead =
		recv(_client->getFd(), _readBuffer.data(), _readBuffer.size(), 0);

	if (bytesRead > 0) {
		_context.recvBuffer.append(_readBuffer.data(), bytesRead);
		handleRequest();
		_client->updateTimeout(); // データ受信時にタイムアウトをリセット
	} else if (bytesRead == 0) {
		LOG(INFO) << "Client disconnected gracefully"
				  << attr("fd", _client->getFd());
		_eventHandler.onConnectionClose(_client->getFd());
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "recv() failed" << attr("fd", _client->getFd())
					   << attr("error", strerror(errno));
			_eventHandler.onConnectionClose(_client->getFd());
		}
	}
}

void HttpConnection::handleWriteEvent() {
	const std::string &sendBuffer =
		_client->getSocket().getSendBuffer(); // getSocket()は参照を返す
	if (sendBuffer.empty()) {
		_eventHandler.onSocketModify(_client->getFd(), EPOLLIN);
		return;
	}

	const ssize_t bytesSent =
		send(_client->getFd(), sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		// getSocket()は参照を返す
		_client->getSocket().eraseSendBuffer(0, bytesSent);

		if (_client->getSocket().getSendBuffer().empty()) {
			// 送信完了
			PipelineContext &ctx = _context;
			if (ctx.res.getHeader("Connection") == "close") {
				_eventHandler.onConnectionClose(_client->getFd());
			} else {
				_eventHandler.onSocketModify(_client->getFd(), EPOLLIN);
				_eventHandler.onRequestProcessed(); // reset()の前に呼ぶ
				ctx.reset(_client->getServer().getConfig());
			}
		}
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "send() failed" << attr("fd", _client->getFd())
					   << attr("error", strerror(errno));
			_eventHandler.onConnectionClose(_client->getFd());
		}
	}
}

time_t HttpConnection::calculateTimeout() const {
	RequestParser::ParseState state = getParserState();
	const Config &conf = _context.conf;

	switch (state) {
	case RequestParser::STATE_REQUEST_LINE:
	case RequestParser::STATE_HEADERS:
		return static_cast< time_t >(conf.getRequestHeaderTimeoutSec());
	case RequestParser::STATE_BODY:
		return static_cast< time_t >(conf.getRequestBodyTimeoutSec());
	case RequestParser::STATE_COMPLETE:
	default:
		return static_cast< time_t >(
			conf.getTimeoutSec()); // Keep-Aliveタイムアウト
	}
}

RequestParser::ParseState HttpConnection::getParserState() const {
	return _context.parser.getState();
}

void HttpConnection::handleRequest() {
	// ミドルウェア処理
	_client->getServer().getMainProcessor().handle(_context);

	// CGI開始チェック
	if (_context.isCgi) {
		_eventHandler.onCgiChanges();
		_context.isCgi = false;
		return;
	}

	// パーサー/ミドルウェアがエラーを検出した場合に応答を生成する
	if (_context.parser.isComplete() || _context.parser.getErrorCode() != 0) {
		generateResponse();
	}
}

void HttpConnection::generateResponse() {
	if (!_context.parser.isComplete() && _context.parser.getErrorCode() == 0) {
		return;
	}
	std::string sid = (_context.session ? _context.session->getId() : "");
	AccessLogger::getInstance().log(&_context.req, &_context.res,
									_client->getIp(), _client->getPort(), sid);

	const std::string responseStr = ResponseBuilder::build(_context.res);
	if (!responseStr.empty()) {
		_client->getSocket().setSendBuffer( // getSocket()は参照を返す
			_client->getSocket().getSendBuffer() + responseStr);
	}

	if (!_client->getSocket().getSendBuffer().empty()) {
		_eventHandler.onSocketModify(_client->getFd(), EPOLLIN | EPOLLOUT);
	}
}

void HttpConnection::resetForNextRequest() {
	// 次のリクエストのためにコンテキストをリセット
	_context.reset(_client->getServer().getConfig());
}
