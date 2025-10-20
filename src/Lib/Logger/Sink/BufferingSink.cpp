#include "BufferingSink.hpp"
#include "../AccessLog/AccessLogStructure.hpp"
#include "../ErrorLog/LogStructure.hpp"
#include "../Form/LogForm.hpp"

BufferingSink::BufferingSink(LogForm *form, size_t bufferThreshold,
							 size_t logCountThreshold,
							 std::time_t timeThreshold)
	: LogSink(form), _logCount(0), _lastFlushTime(std::time(NULL)),
	  _bufferThreshold(bufferThreshold), _logCountThreshold(logCountThreshold),
	  _timeThreshold(timeThreshold) {}

BufferingSink::~BufferingSink() {
	// The flush() virtual call will be resolved to the concrete class's
	// implementation
	flush();
}

void BufferingSink::log(const LogMessage &msg) {
	writeToBuffer(&LogForm::format, msg);
}

void BufferingSink::logAccess(const AccessLogContext &ctx) {
	writeToBuffer(&LogForm::formatAccess, ctx);
}

void BufferingSink::flush() { flushBuffer(); }

bool BufferingSink::shouldFlush() {
	if (static_cast< size_t >(_buffer.tellp()) >= _bufferThreshold) {
		return true;
	}
	if (_logCount >= _logCountThreshold) {
		return true;
	}
	std::time_t now = std::time(NULL);
	if (now - _lastFlushTime >= _timeThreshold) {
		return true;
	}
	return false;
}

template < typename T >
void BufferingSink::writeToBuffer(void (LogForm::*formatFunc)(const T &,
															  std::ostream &),
								  const T &data) {
	(this->_form->*formatFunc)(data, _buffer);
	_buffer << '\n';
	_logCount++;

	if (shouldFlush()) {
		flush();
	}
}

// Explicit template instantiation to avoid linker errors
template void BufferingSink::writeToBuffer< LogMessage >(
	void (LogForm::*formatFunc)(const LogMessage &, std::ostream &),
	const LogMessage &data);
template void BufferingSink::writeToBuffer< AccessLogContext >(
	void (LogForm::*formatFunc)(const AccessLogContext &, std::ostream &),
	const AccessLogContext &data);
