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

#define _DEFAULT_LOG_DIR "./log"

class Logger {
public:
	static Logger &getInstance();
	static Logger &getInstance(const std::string& logDir);
	static void cleanup(); // プログラム終了時にリソースを解放

	void addSink(LogSink *sink);
	void log(const LogMessage &msg);

private:
	Logger(const std::string& logDir = "./log");
	~Logger();
	Logger(const Logger &);
	Logger &operator=(const Logger &);

	static Logger *_instance;
	std::vector<LogSink *> _sinks;
	std::string _logDir;

	static const size_t _maxLogFileSize = (10 * 1024 * 1024); // 10MB
	static const size_t _maxBackupFiles = 8; // 最大バックアップファイル数
};
