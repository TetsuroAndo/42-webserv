#include "WriteEvent.hpp"

#include "../../Server.hpp"

WriteEvent::WriteEvent(Client *client)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLOUT) {}

WriteEvent::~WriteEvent() {}

void WriteEvent::handle() { _httpConnection.handleWriteEvent(); }

void WriteEvent::close() {}
