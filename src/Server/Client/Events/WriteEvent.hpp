#pragma once
#include "AEvent.hpp"

class WriteEvent : public AEvent {
public:
	WriteEvent(Client *client);
	~WriteEvent();

	void handle();
	void close();

private:
};
