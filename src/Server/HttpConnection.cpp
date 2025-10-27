#include "HttpConnection.hpp"
#include "../Http/Builder/ResponseBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <cerrno>
#include <cstring>
#include <unistd.h>

HttpConnection::HttpConnection(Client *client, PipelineContext *context,
							   HttpConnectionEventHandler *eventHandler)
	: _client(client), _context(context), _eventHandler(eventHandler) {}

HttpConnection::~HttpConnection() {}

void HttpConnection::processRequest() {
	parseRequest();
	generateResponse();
}

void HttpConnection::handleReadEvent() {
	char buffer[4096];
	const ssize_t bytesRead = recv(_client->getFd(), buffer, sizeof(buffer), 0);

	if (bytesRead > 0) {
		_context->recvBuffer.append(buffer, bytesRead);
		parseRequest();
	} else if (bytesRead == 0) {
		LOG(INFO) << "Client disconnected gracefully"
				  << attr("fd", _client->getFd());
		if (_eventHandler) {
			_eventHandler->onConnectionClose(_client->getFd());
		}
		return;
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "recv() failed" << attr("fd", _client->getFd())
					   << attr("error", strerror(errno));
			if (_eventHandler) {
				_eventHandler->onConnectionClose(_client->getFd());
			}
		}
		return;
	}
}

void HttpConnection::handleWriteEvent() {
	const std::string &sendBuffer = _client->getSocket()->getSendBuffer();
	if (sendBuffer.empty()) {
		if (_eventHandler) {
			_eventHandler->onSocketModify(_client->getFd(), EPOLLIN);
		}
		return;
	}

	const ssize_t bytesSent =
		send(_client->getFd(), sendBuffer.c_str(), sendBuffer.size(), 0);

	if (bytesSent > 0) {
		_client->getSocket()->eraseSendBuffer(0, bytesSent);

		if (_client->getSocket()->getSendBuffer().empty()) {
			// 送信完了
			PipelineContext *ctx = _context;
			if (ctx->res.getHeader("Connection") == "close") {
				if (_eventHandler) {
					_eventHandler->onConnectionClose(_client->getFd());
				}
			} else {
				if (_eventHandler) {
					_eventHandler->onSocketModify(_client->getFd(), EPOLLIN);
				}
				ctx->reset(_client->getServer()->getConfig());
				if (_eventHandler) {
					_eventHandler->onRequestProcessed();
				}
			}
		}
	} else {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			LOG(ERROR) << "send() failed" << attr("fd", _client->getFd())
					   << attr("error", strerror(errno));
			if (_eventHandler) {
				_eventHandler->onConnectionClose(_client->getFd());
			}
		}
	}
}

time_t HttpConnection::calculateTimeout() const {
	RequestParser::ParseState state = getParserState();
	const Config &conf = _context->conf;

	switch (state) {
	case RequestParser::STATE_REQUEST_LINE:
	case RequestParser::STATE_HEADERS:
		return conf.getRequestHeaderTimeoutSec();
	case RequestParser::STATE_BODY:
		return conf.getRequestBodyTimeoutSec();
	case RequestParser::STATE_COMPLETE:
	default:
		return conf.getTimeoutSec(); // Keep-Aliveタイムアウト
	}
}

RequestParser::ParseState HttpConnection::getParserState() const {
	return _context->parser.getState();
}

void HttpConnection::parseRequest() {
	// ミドルウェア処理
	_client->getServer()->getMainProcessor().handle(*_context);

	// CGI開始チェック
	if (_context->isCgi) {
		if (_eventHandler) {
			_eventHandler->onCgiChanges();
		}
		_context->isCgi = false;
		return;
	}
}

void HttpConnection::generateResponse() {
	if (_context->parser.isComplete() || _context->parser.getErrorCode() != 0) {
		std::string sid = (_context->session ? _context->session->getId() : "");
		AccessLogger::getInstance().log(&_context->req, &_context->res,
										_client->getIp(), _client->getPort(),
										sid);

		const std::string responseStr = ResponseBuilder::build(_context->res);
		if (!responseStr.empty()) {
			_client->getSocket()->setSendBuffer(
				_client->getSocket()->getSendBuffer() + responseStr);
		}

		if (!_client->getSocket()->getSendBuffer().empty()) {
			if (_eventHandler) {
				_eventHandler->onSocketModify(_client->getFd(),
											  EPOLLIN | EPOLLOUT);
			}
		}
	}
}

void HttpConnection::resetForNextRequest() {
	// 次のリクエストのためにコンテキストをリセット
	_context->reset(_client->getServer()->getConfig());
}
