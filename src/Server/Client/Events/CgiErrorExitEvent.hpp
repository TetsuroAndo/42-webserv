#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiErrorExitEvent : public AEvent, public ACgiEvent {
public:
	CgiErrorExitEvent(Client *client, CgiWorker &worker);
	~CgiErrorExitEvent();

	void handle();
	void process();

private:
};
