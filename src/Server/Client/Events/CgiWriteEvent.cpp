#include "CgiWriteEvent.hpp"

CgiWriteEvent::CgiWriteEvent(Client *client)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLOUT) {}

CgiWriteEvent::~CgiWriteEvent() {}

void CgiWriteEvent::handle() {}

void CgiWriteEvent::close() {}
