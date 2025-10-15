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

class AccessLogger {
public:
	static AccessLogger &getInstance();

	void setLogDir(const std::string &logDir);
	void setSinkFile(const std::string &filename, LogFormat eFormat,
					 size_t maxFileSize, size_t maxBackupFiles);
	void setSinkFile(const std::string &logDir, const std::string &filename,
					 LogFormat eFormat, size_t maxFileSize,
					 size_t maxBackupFiles);
	void setSinkConsole(LogFormat eFormat);

	void log(const AccessLogContext &ctx);
	void log(const HttpRequest *request, const HttpResponse *response,
			 std::string remote_addr, int client_port, std::string session_id);

private:
	AccessLogger();
	~AccessLogger();
	AccessLogger(const AccessLogger &);
	AccessLogger &operator=(const AccessLogger &);

	void addSink(LogSink *sink);

	std::string _logDir;
	std::vector< LogSink * > _sinks;
};
