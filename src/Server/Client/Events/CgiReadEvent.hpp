#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiReadEvent : public AEvent, public ACgiEvent {
public:
	CgiReadEvent(CgiWorker &worker);
	~CgiReadEvent();

	void handle();
	void process();

private:
};
