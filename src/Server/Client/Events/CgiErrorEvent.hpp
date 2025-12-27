#pragma once
#include "AEvent.hpp"

class CgiErrorEvent : public AEvent {
public:
	CgiErrorEvent(Client *client);
	~CgiErrorEvent();

	void handle();
	void close();

private:
};
