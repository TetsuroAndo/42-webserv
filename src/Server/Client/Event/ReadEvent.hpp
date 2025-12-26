#pragma once
#include "AEvent.hpp"

class ReadEvent : public AEvent {
public:
	ReadEvent();
	~ReadEvent();

	void handle();

private:
};
