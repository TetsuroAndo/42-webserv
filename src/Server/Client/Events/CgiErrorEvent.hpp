#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiErrorEvent : public AEvent, public ACgiEvent {
public:
	CgiErrorEvent(Client *client, CgiWorker &worker);
	~CgiErrorEvent();

	void handle();
	void process();

private:
};
