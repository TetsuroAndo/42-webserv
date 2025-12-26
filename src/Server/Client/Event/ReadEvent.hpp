#pragma once
#include "AEvent.hpp"

class ReadEvent : public AEvent {
public:
	ReadEvent(Client *client, PipelineContext &context,
			  HttpConnection &httpConnection);
	~ReadEvent();

	void handle();

private:
};
