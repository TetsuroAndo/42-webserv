#include "../../Time/TimeCache.hpp"
#include "LogBuilder.hpp"
#include "Logger.hpp"
#include <ctime>

LogBuilder::LogBuilder(const LogLevel level, const char *file, const int line,
					   const char *func) {
	_msg.localDate = TimeCache::getLocalDate();
	_msg.localTime = TimeCache::getLocalTime();
	_msg.level = level;
	_msg.file = file;
	_msg.line = line;
	_msg.function = func;
}

LogBuilder::~LogBuilder() {
	_msg.message = _ss.str();
	Logger::getInstance().log(_msg);
}

LogBuilder &operator<<(LogBuilder &builder, const LogAttribute &attr) {
	builder._msg.attributes[attr.key] = attr.value;
	return builder;
}
