#include "ReadEvent.hpp"

ReadEvent::ReadEvent(Client *client, PipelineContext &context,
					 HttpConnection &httpConnection)
	: AEvent(client, context, httpConnection) {}

ReadEvent::~ReadEvent() {}

void ReadEvent::handle() { _httpConnection.handleReadEvent(); }
