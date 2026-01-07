#pragma once
#include "ACgiEvent.hpp"
#include "AEvent.hpp"

class CgiWriteEvent : public AEvent, public ACgiEvent {
public:
	CgiWriteEvent(CgiWorker &worker);
	~CgiWriteEvent();

	void handle();
	void process();

private:
};
