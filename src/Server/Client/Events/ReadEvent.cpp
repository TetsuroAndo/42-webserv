#include "ReadEvent.hpp"

#include "../../Server.hpp"

ReadEvent::ReadEvent(Client *client) : AEvent(client, EPOLLIN) {}

ReadEvent::~ReadEvent() {}

void ReadEvent::handle() { _client->getHttpConnection().handleReadEvent(); }
