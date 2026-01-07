#include "WriteEvent.hpp"

#include "../../Server.hpp"

WriteEvent::WriteEvent(Client *client) : AEvent(client, EPOLLOUT) {}

WriteEvent::~WriteEvent() {}

void WriteEvent::handle() { _client->getHttpConnection().handleWriteEvent(); }
