#include "CgiEndEvent.hpp"

CgiEndEvent::CgiEndEvent(Client *client)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN) {}

CgiEndEvent::~CgiEndEvent() {}

void CgiEndEvent::handle() {}

void CgiEndEvent::close() {}
