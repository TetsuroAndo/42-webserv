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
	AEvent(const unsigned int expectedEventType)
		: _client(NULL), _expectedEventType(expectedEventType), _fd(-1) {}
	AEvent(Client *client, const unsigned int expectedEventType)
		: _client(client), _expectedEventType(expectedEventType), _fd(-1) {
		if (client == NULL) {
			throw std::runtime_error("AEvent::AEvent(): client is NULL");
		}
	}
	virtual ~AEvent() {}

	virtual void handle() = 0;

	bool isExpectedEventType(const uint32_t event) const {
		return (event & _expectedEventType) != 0;
	}
	void setFd(const int fd) { _fd = fd; }
	int getFd() const { return _fd; }

protected:
	Client *_client;
	const unsigned int _expectedEventType;
	int _fd;
};
