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

#define _LOG_DEFAULT_DIR "./log"
#define _LOG_FALLBACK_DIR                                                      \
	"./" // ディレクトリが存在しな場合はルートディレクトリに作成
#define _LOG_MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB
#define _LOG_MAX_BACKUPS 8

class Logger {
public:
	static Logger &getInstance();

	void setLogDir(const std::string &logDir);
	void setSinkFile(const std::string &filename, LogFormat eFormat,
					 LogLevel level, LogFilterMode mode = GREATER_OR_EQUAL,
					 size_t maxFileSize = _LOG_MAX_FILE_SIZE,
					 size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void setSinkFile(const std::string &logDir, const std::string &filename,
					 LogFormat eFormat, LogLevel level,
					 LogFilterMode mode = GREATER_OR_EQUAL,
					 size_t maxFileSize = _LOG_MAX_FILE_SIZE,
					 size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void setSinkConsole(LogFormat eFormat, LogLevel level,
						LogFilterMode mode = GREATER_OR_EQUAL);

	void log(const LogMessage &msg);
	bool isLogLevelActive(LogLevel level) const;

private:
	Logger();
	~Logger();
	Logger(const Logger &);
	Logger &operator=(const Logger &);

	void addSink(LogLevel level, LogFilterMode mode, LogSink* sink);
	void updateActiveLevelsMask();

	static const int NUM_LOG_LEVELS = FATAL + 1;

	std::string _logDir;
	std::vector<LogSink *> _sinksByLevel[NUM_LOG_LEVELS];
	std::vector<LogSink *> _ownedSinks;
	unsigned int _activeLevelsMask;
};
