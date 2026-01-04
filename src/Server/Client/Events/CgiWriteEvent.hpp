#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiWriteEvent : public AEvent, public ACgiEvent {
public:
	CgiWriteEvent(Client *client, CgiWorker &worker);
	~CgiWriteEvent();

	void handle();
	void process();

private:
};
