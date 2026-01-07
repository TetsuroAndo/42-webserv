#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiEndEvent : public AEvent, public ACgiEvent {
public:
	CgiEndEvent(Client *client, CgiWorker *worker);
	~CgiEndEvent();

	void handle();
	void process();

private:
};
