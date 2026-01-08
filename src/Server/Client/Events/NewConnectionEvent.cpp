#include "NewConnectionEvent.hpp"

NewConnectionEvent::NewConnectionEvent(INewConnectionHandler &handler)
	: AEvent(EPOLLIN), _handler(handler) {}

NewConnectionEvent::~NewConnectionEvent() {}

void NewConnectionEvent::handle() { _handler.handleNewConnection(_fd); }
