#pragma once

#include "LogSink.hpp"
#include <string>
#include <vector>

struct LogMessage;
enum LogLevel { DEBUG, INFO, WARNING, ERROR, FATAL };

class Logger {
public:
	static Logger &getInstance();
	static void cleanup(); // プログラム終了時にリソースを解放

	void addSink(LogSink *sink);
	void setLogLevel(LogLevel level);
	void log(const LogMessage &msg);

private:
	Logger();
	~Logger();
	Logger(const Logger &);
	Logger &operator=(const Logger &);

	std::string levelToString(LogLevel level) const;
	std::string formatMessage(const LogMessage &msg);

	static Logger *_instance;
	LogLevel _logLevel;
	std::vector<LogSink *> _sinks;
};
