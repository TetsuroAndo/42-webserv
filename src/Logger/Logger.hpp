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

class Logger {
public:
	static Logger &getInstance();
	static Logger &getInstance(const std::string& logDir);
	static void cleanup(); // プログラム終了時にリソースを解放

	void addSink(LogSink *sink);
	void addFileSink(const std::string &filename, const std::string &Form, LogLevel level,
		size_t maxFileSize = _LOG_MAX_FILE_SIZE, size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void addFileSink(const std::string &logDir, const std::string &filename,
		const std::string &Form, LogLevel level, size_t maxFileSize = _LOG_MAX_FILE_SIZE,
		size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void addConsoleSink(const std::string &Form, LogLevel level);

	void log(const LogMessage &msg);

private:
	Logger(const std::string& logDir = _DEFAULT_LOG_DIR);
	~Logger();
	Logger(const Logger &);
	Logger &operator=(const Logger &);

	static Logger *_instance;
	std::vector<LogSink *> _sinks;
	std::string _logDir;
};
