#pragma once

#include "BufferingSink.hpp"
#include <string>

class ConsoleSink : public BufferingSink {
public:
	ConsoleSink(LogForm *form);
	virtual ~ConsoleSink();

private:
	virtual void flushBuffer();
};
