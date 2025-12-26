#pragma once
#include "AEvent.hpp"

class WriteEvent : public AEvent {
public:
	WriteEvent(Client *client, PipelineContext &context,
			   HttpConnection &httpConnection);
	~WriteEvent();

	void handle();

private:
};
