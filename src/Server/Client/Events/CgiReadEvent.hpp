#pragma once
#include "AEvent.hpp"

class CgiReadEvent : public AEvent {
public:
	CgiReadEvent(Client *client);
	~CgiReadEvent();

	void handle();
	void close();

private:
};
