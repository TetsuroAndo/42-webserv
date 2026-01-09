#pragma once

#include "../../Listen/INewConnectionHandler.hpp"
#include "AEvent.hpp"

class NewConnectionEvent : public AEvent {
public:
	NewConnectionEvent(INewConnectionHandler &handler);
	~NewConnectionEvent();

	void handle();

private:
	INewConnectionHandler &_handler;
};
