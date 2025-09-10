#include "Logger.hpp"
#include "LogMessage.hpp"
#include <ctime>
#include <sstream>
#include <stdexcept>

Logger* Logger::_instance = NULL;

Logger& Logger::getInstance() {
	if (_instance == NULL) {
		_instance = new Logger();
	}
	return *_instance;
}

void Logger::cleanup() {
	delete _instance;
	_instance = NULL;
}

Logger::Logger() : _logLevel(DEBUG) {}

Logger::~Logger() {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		delete *it;
	}
	_sinks.clear();
}

void Logger::addSink(LogSink* sink) {
	_sinks.push_back(sink);
}

void Logger::setLogLevel(LogLevel level) {
	_logLevel = level;
}

void Logger::log(const LogMessage& msg) {
	if (msg.level < _logLevel) {
		return;
	}

	std::string formatted = formatMessage(msg);
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		(*it)->write(formatted);
	}
}

std::string Logger::levelToString(LogLevel level) const {
	switch (level) {
		case DEBUG: return "DEBUG";
		case INFO: return "INFO";
		case WARNING: return "WARNING";
		case ERROR: return "ERROR";
		case FATAL: return "FATAL";
		default: return "UNKNOWN";
	}
}

std::string Logger::formatMessage(const LogMessage& msg) {
	std::stringstream ss;
	char timeStr[20];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S",
			 localtime(&msg.timestamp));

	ss << "{";
	ss << "\"timestamp\":\"" << timeStr << "\",";
	ss << "\"level\":\"" << levelToString(msg.level) << "\",";

	// JSON文字列内の特殊文字をエスケープ
	std::string escapedMessage = msg.message;
	size_t pos = 0;
	while ((pos = escapedMessage.find("\"", pos)) != std::string::npos) {
		escapedMessage.replace(pos, 1, "\\\"");
		pos += 2;
	}
	ss << "\"message\":\"" << escapedMessage << "\",";

	ss << "\"source\":\"" << msg.file << ":" << msg.line << "\"";

	if (!msg.attributes.empty()) {
		ss << ",\"attributes\":{";
		for (std::map<std::string, std::string>::const_iterator it =
				 msg.attributes.begin();
			 it != msg.attributes.end();) {
			ss << "\"" << it->first << "\":\"" << it->second << "\"";
			if (++it != msg.attributes.end()) {
				ss << ",";
			}
		}
		ss << "}";
	}
	ss << "}";
	return ss.str();
}
