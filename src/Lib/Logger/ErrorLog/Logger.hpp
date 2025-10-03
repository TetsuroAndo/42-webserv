#pragma once

#include "../Form/ElfForm.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/LogForm.hpp"
#include "../LogType.hpp"
#include "../Sink/ConsoleSink.hpp"
#include "../Sink/FileSink.hpp"
#include "../Sink/LogSink.hpp"
#include "LogBuilder.hpp"
#include <map>
#include <string>
#include <vector>

class Logger {
public:
	static Logger &getInstance();

	void setLogDir(const std::string &logDir);
	void setSinkFile(const std::string &filename, LogFormat eFormat,
					 LogLevel level, LogFilterMode mode, size_t maxFileSize,
					 size_t maxBackupFiles);
	void setSinkFile(const std::string &logDir, const std::string &filename,
					 LogFormat eFormat, LogLevel level, LogFilterMode mode,
					 size_t maxFileSize, size_t maxBackupFiles);
	void setSinkConsole(LogFormat eFormat, LogLevel level, LogFilterMode mode);

	void log(const LogMessage &msg);
	bool isLogLevelActive(LogLevel level) const;

private:
	Logger();
	~Logger();
	Logger(const Logger &);
	Logger &operator=(const Logger &);

	void addSink(LogLevel level, LogFilterMode mode, LogSink *sink);
	void updateActiveLevelsMask();

	static const int NUM_LOG_LEVELS = FATAL + 1;

	std::string _logDir;
	std::vector<LogSink *> _sinksByLevel[NUM_LOG_LEVELS];
	std::vector<LogSink *> _ownedSinks;
	unsigned int _activeLevelsMask;
};
