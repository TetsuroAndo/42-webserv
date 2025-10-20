#include "ConsoleSink.hpp"
#include <iostream>

namespace {
const size_t BUFFER_THRESHOLD = 4096; // 4KB
const size_t LOG_COUNT_THRESHOLD = 50;
const std::time_t TIME_THRESHOLD = 3;
} // namespace

ConsoleSink::ConsoleSink(LogForm *form)
	: BufferingSink(form, BUFFER_THRESHOLD, LOG_COUNT_THRESHOLD,
					TIME_THRESHOLD) {}

// Destructor is virtual and empty, base class destructor handles flushing
ConsoleSink::~ConsoleSink() {}

void ConsoleSink::flushBuffer() {
	if (static_cast< size_t >(_buffer.tellp()) > 0) {
		std::cout << _buffer.str();
		std::cout.flush();

		// Clear buffer and reset counters
		_buffer.str("");
		_buffer.clear();
		_logCount = 0;
		_lastFlushTime = std::time(NULL);
	}
}
