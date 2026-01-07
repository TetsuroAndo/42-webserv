#pragma once
#include "../../Server.hpp"
#include "AEvent.hpp"

class NewConnectionEvent : public AEvent {
public:
	NewConnectionEvent(Client *client, Server &server);
	~NewConnectionEvent();

	void handle();

private:
	Server &_server;
};
