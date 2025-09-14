#pragma once

#include "Form/LogForm.hpp"
#include "Form/JsonForm.hpp"
#include "Form/ElfForm.hpp"
#include "LogBuilder.hpp"
#include "LogStructure.hpp"
#include "Sink/LogSink.hpp"
#include "Sink/ConsoleSink.hpp"
#include "Sink/FileSink.hpp"
#include <string>
#include <vector>

#define _LOG_DEFAULT_DIR "./log"
#define _LOG_FALLBACK_DIR "./" // ディレクトリが存在しな場合はルートディレクトリに作成
#define _LOG_MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB
#define _LOG_MAX_BACKUPS 8

class Logger {
public:
	static Logger &getInstance();

	void setLogDir(const std::string &logDir);
	void setSinkFile(const std::string &filename, LogFormat eFormat,
		LogLevel level, LogFilterMode mode = GREATER_OR_EQUAL,
		size_t maxFileSize = _LOG_MAX_FILE_SIZE, size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void setSinkFile(const std::string &logDir, const std::string &filename,
		LogFormat eFormat, LogLevel level, LogFilterMode mode = GREATER_OR_EQUAL,
		size_t maxFileSize = _LOG_MAX_FILE_SIZE, size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void setSinkConsole(LogFormat eFormat, LogLevel level, LogFilterMode mode = GREATER_OR_EQUAL);

	void log(const LogMessage &msg);
	bool isLogLevelActive(LogLevel level) const;

private:
	Logger();
	~Logger();
	Logger(const Logger &);
	Logger &operator=(const Logger &);

	void updateActiveLevelsMask();

	std::string _logDir;
	std::vector<LogSink *> _sinks;
	unsigned int _activeLevelsMask;
};
