#pragma once

#include "LogSink.hpp"
#include <ctime>
#include <sstream>

class BufferingSink : public LogSink {
public:
	BufferingSink(LogForm *form, size_t bufferThreshold,
				  size_t logCountThreshold, std::time_t timeThreshold);
	virtual ~BufferingSink();

	virtual void log(const LogMessage &msg);
	virtual void logAccess(const AccessLogContext &ctx);
	virtual void flush();

protected:
	// This is the method concrete sinks must implement
	virtual void flushBuffer() = 0;

	bool shouldFlush();

	template < typename T >
	void writeToBuffer(void (LogForm::*formatFunc)(const T &, std::ostream &),
					   const T &data);

	std::ostringstream _buffer;
	size_t _logCount;
	std::time_t _lastFlushTime;

	const size_t _bufferThreshold;
	const size_t _logCountThreshold;
	const std::time_t _timeThreshold;
};
