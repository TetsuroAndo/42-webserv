#pragma once
#include "../Client.hpp"

/**
 * @class AEvent
 * @brief イベント処理
 * epollのもつfdで管理される、Clientが発火するべきイベントを抽象的に利用するためのクラス
 */
class AEvent {
public:
	AEvent(Client *client, PipelineContext &context,
		   HttpConnection &httpConnection);
	virtual ~AEvent();

	virtual void handle() = 0;

protected:
	Client &_client;
	PipelineContext &_context;
	HttpConnection &_httpConnection;
};
