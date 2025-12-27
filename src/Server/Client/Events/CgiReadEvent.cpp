#include "CgiReadEvent.hpp"

CgiReadEvent::CgiReadEvent(Client *client)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN) {}

CgiReadEvent::~CgiReadEvent() {}

void CgiReadEvent::handle() {}

void CgiReadEvent::close() {}
