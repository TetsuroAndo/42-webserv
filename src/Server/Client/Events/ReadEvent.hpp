#pragma once
#include "AEvent.hpp"

class ReadEvent : public AEvent {
public:
	ReadEvent(Client *client);
	~ReadEvent();

	void handle();
	void close();

private:
};
