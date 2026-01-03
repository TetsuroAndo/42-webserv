#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiReadEvent : public AEvent, public ACgiEvent {
public:
	CgiReadEvent(Client *client, CgiWorker &worker);
	~CgiReadEvent();

	void handle();
	void process();
	void close();

private:
};
