#pragma once

#include "../Form/ElfForm.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/LogForm.hpp"
#include "../LogType.hpp"
#include "../Sink/ConsoleSink.hpp"
#include "../Sink/FileSink.hpp"
#include "../Sink/LogSink.hpp"
#include <string>
#include <vector>

#define _LOG_DEFAULT_DIR "./log"
#define _LOG_FALLBACK_DIR                                                      \
	"./" // ディレクトリが存在しな場合はルートディレクトリに作成
#define _LOG_MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB
#define _LOG_MAX_BACKUPS 8

class AccessLogger {
public:
	static AccessLogger &getInstance();

	void setLogDir(const std::string &logDir);
	void setSinkFile(const std::string &filename, LogFormat eFormat,
					 size_t maxFileSize = _LOG_MAX_FILE_SIZE,
					 size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void setSinkFile(const std::string &logDir, const std::string &filename,
					 LogFormat eFormat,	 size_t maxFileSize = _LOG_MAX_FILE_SIZE,
					 size_t maxBackupFiles = _LOG_MAX_BACKUPS);
	void setSinkConsole(LogFormat eFormat);

	void log(const AccessLogContext &ctx);
	void log(const HttpRequest* request,
			 const HttpResponse* response,
			 std::string remote_addr,
			 int client_port,
			 std::string session_id);

private:
	AccessLogger();
	~AccessLogger();
	AccessLogger(const AccessLogger &);
	AccessLogger &operator=(const AccessLogger &);

	void addSink(LogSink* sink);

	std::string _logDir;
	std::vector<LogSink *> _sinks;
};
