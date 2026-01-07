#include "NewConnectionEvent.hpp"

#include "../../Server.hpp"

NewConnectionEvent::NewConnectionEvent(Server &server)
	: AEvent(EPOLLIN), _server(server) {}

NewConnectionEvent::~NewConnectionEvent() {}

void NewConnectionEvent::handle() { _server.handleNewConnection(_fd); }
