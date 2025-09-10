#include "LogBuilder.hpp"
#include "Logger.hpp"
#include <ctime>

LogBuilder::LogBuilder(LogLevel level, const char* file, int line) {
	_msg.timestamp = std::time(NULL);
	_msg.level = level;
	_msg.file = file;
	_msg.line = line;
}

LogBuilder::~LogBuilder() {
	_msg.message = _ss.str();
	Logger::getInstance().log(_msg);
}

LogBuilder& LogBuilder::addAttribute(const std::string& key, const std::string& value) {
	_msg.attributes[key] = value;
	return *this;
}
