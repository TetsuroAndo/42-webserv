#pragma once
#include "AEvent.hpp"

class WriteEvent : public AEvent {
public:
	WriteEvent();
	~WriteEvent();

	void handle();

private:
};
