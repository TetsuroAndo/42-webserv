#include "AccessLogger.hpp"
#include "../../Time/TimeCache.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>

AccessLogger &AccessLogger::getInstance() {
	static AccessLogger instance;
	return instance;
}

void AccessLogger::setLogDir(const std::string &logDir) {
	struct stat st;
	if (stat(logDir.c_str(), &st) != 0) {
		std::cerr
			<< "[ WARNING ] AccessLogger: Log directory does not exist: " +
				   logDir
			<< ", using default directory: " << _logDir << std::endl;
	} else if (!S_ISDIR(st.st_mode)) {
		std::cerr << "[ WARNING ] AccessLogger: Log path exists but is not a "
					 "directory: " +
						 logDir
				  << ", using default directory: " << _logDir << std::endl;
	} else {
		_logDir = logDir;
	}
}

AccessLogger::AccessLogger() : _logDir(_LOG_DEFAULT_DIR) {
	struct stat st;
	if (stat(_logDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
		_logDir = _LOG_FALLBACK_DIR;
		std::cerr
			<< "[ ERROR ] AccessLogger: Default log directory does not exist,"
			<< " using fallback directory: " << _logDir << std::endl;
	}
}

AccessLogger::~AccessLogger() {
	for (std::vector< LogSink * >::iterator it = _sinks.begin();
		 it != _sinks.end(); ++it) {
		delete *it;
	}
	_sinks.clear();
}

void AccessLogger::setSinkFile(const std::string &filename,
							   const LogFormat eFormat,
							   const size_t maxFileSize,
							   const size_t maxBackupFiles) {
	LogForm *form = NULL;
	FileSink *sink = NULL;
	try {
		if (eFormat == JSON) {
			form = new JsonForm();
		} else {
			form = new ElfForm();
		}
		sink =
			new FileSink(_logDir, filename, form, maxFileSize, maxBackupFiles);
		addSink(sink);
	} catch (...) {
		delete sink;
		delete form;
		throw;
	}
}

void AccessLogger::setSinkFile(const std::string &logDir,
							   const std::string &filename,
							   const LogFormat eFormat,
							   const size_t maxFileSize,
							   const size_t maxBackupFiles) {
	LogForm *form = NULL;
	FileSink *sink = NULL;
	try {
		if (eFormat == JSON) {
			form = new JsonForm();
		} else {
			form = new ElfForm();
		}
		sink =
			new FileSink(logDir, filename, form, maxFileSize, maxBackupFiles);
		addSink(sink);
	} catch (...) {
		delete sink;
		delete form;
		throw;
	}
}

void AccessLogger::setSinkConsole(const LogFormat eFormat) {
	LogForm *form = NULL;
	ConsoleSink *sink = NULL;
	try {
		if (eFormat == JSON) {
			form = new JsonForm();
		} else {
			form = new ElfForm();
		}
		sink = new ConsoleSink(form);
		addSink(sink);
	} catch (...) {
		delete sink;
		delete form;
		throw;
	}
}

void AccessLogger::log(const AccessLogContext &ctx) {
	for (std::vector< LogSink * >::iterator it = _sinks.begin();
		 it != _sinks.end(); ++it) {
		try {
			(*it)->logAccess(ctx);
		} catch (const std::exception &e) {
			std::cerr << "[ ERROR ] AccessLogger: Failed to write log: "
					  << e.what() << std::endl;
		}
	}
}

void AccessLogger::log(const HttpRequest *request, const HttpResponse *response,
					   std::string remote_addr, int client_port,
					   std::string session_id) {
	AccessLogContext ctx = {TimeCache::getUtcTimestamp(),
							TimeCache::getIsoTimestamp(),
							request,
							response,
							remote_addr,
							client_port,
							session_id};
	this->log(ctx);
}

void AccessLogger::addSink(LogSink *sink) { _sinks.push_back(sink); }
