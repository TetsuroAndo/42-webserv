#include "ReadEvent.hpp"

#include "../../Server.hpp"

ReadEvent::ReadEvent(Client *client)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN) {}

ReadEvent::~ReadEvent() {}

void ReadEvent::handle() { _httpConnection.handleReadEvent(); }

void ReadEvent::close() {}
