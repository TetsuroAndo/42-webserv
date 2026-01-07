#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiErrorExitEvent : public AEvent, public ACgiEvent {
public:
	CgiErrorExitEvent(CgiWorker *worker);
	~CgiErrorExitEvent();

	void handle();
	void process();

private:
};
