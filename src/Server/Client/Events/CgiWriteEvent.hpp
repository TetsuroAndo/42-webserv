#pragma once
#include "AEvent.hpp"

class CgiWriteEvent : public AEvent {
public:
	CgiWriteEvent(Client *client);
	~CgiWriteEvent();

	void handle();
	void close();

private:
};
