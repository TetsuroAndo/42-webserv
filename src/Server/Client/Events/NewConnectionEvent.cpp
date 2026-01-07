#include "NewConnectionEvent.hpp"

#include "../../Server.hpp"

NewConnectionEvent::NewConnectionEvent(Client *client, Server &server)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN),
	  _server(server) {}

NewConnectionEvent::~NewConnectionEvent() {}

void NewConnectionEvent::handle() { _server.handleNewConnection(_fd); }
