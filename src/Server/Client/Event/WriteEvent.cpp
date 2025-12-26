#include "WriteEvent.hpp"

WriteEvent::WriteEvent(Client *client, PipelineContext &context,
					   HttpConnection &httpConnection)
	: AEvent(client, context, httpConnection) {}

WriteEvent::~WriteEvent() {}

void WriteEvent::handle() { _httpConnection.handleWriteEvent(); }
