#pragma once
#include "AEvent.hpp"

class CgiEndEvent : public AEvent {
public:
	CgiEndEvent(Client *client);
	~CgiEndEvent();

	void handle();
	void close();

private:
};
