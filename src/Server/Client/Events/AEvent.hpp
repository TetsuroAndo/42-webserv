#pragma once
#include "../Client.hpp"
#include <sys/epoll.h>

class Client;
struct PipelineContext;
class HttpConnection;
/**
 * @class AEvent
 * @brief イベント処理
 * epollのもつfdで管理される、Clientが発火するべきイベントを抽象的に利用するためのクラス
 */
class AEvent {
public:
	AEvent(Client *client, PipelineContext &context,
		   HttpConnection &httpConnection, const unsigned int expectedEventType)
		: _client(*client), _context(context), _httpConnection(httpConnection),
		  _expectedEventType(expectedEventType), _fd(-1) {}
	virtual ~AEvent() {}

	virtual void handle() = 0;
	virtual void close() = 0;

	bool isExpectedEventType(const uint32_t event) const {
		return (event & _expectedEventType) != 0;
	}
	void setFd(const int fd) { _fd = fd; }
	int getFd() const { return _fd; }

protected:
	Client &_client;
	PipelineContext &_context;
	HttpConnection &_httpConnection;
	const unsigned int _expectedEventType;
	int _fd;
};
