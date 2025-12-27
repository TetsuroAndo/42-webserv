#include "CgiErrorEvent.hpp"

CgiErrorEvent::CgiErrorEvent(Client *client)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN) {}

CgiErrorEvent::~CgiErrorEvent() {}

void CgiErrorEvent::handle() {}

void CgiErrorEvent::close() {}
