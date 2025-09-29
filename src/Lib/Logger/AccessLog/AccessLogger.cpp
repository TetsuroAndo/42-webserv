#include "AccessLogger.hpp"
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
		std::cerr << "[ WARNING ] AccessLogger: Log directory does not exist: " +
						 logDir
				  << ", using default directory: " << _logDir << std::endl;
	} else if (!S_ISDIR(st.st_mode)) {
		std::cerr
			<< "[ WARNING ] AccessLogger: Log path exists but is not a directory: " +
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
		std::cerr << "[ ERROR ] AccessLogger: Default log directory does not exist,"
				  << " using fallback directory: " << _logDir << std::endl;
	}
}

AccessLogger::~AccessLogger() {
	for (std::vector<LogSink *>::iterator it = _sinks.begin();
		 it != _sinks.end(); ++it) {
		delete *it;
	}
	_sinks.clear();
}

void AccessLogger::setSinkFile(const std::string &filename, const LogFormat eFormat,
						 const size_t maxFileSize,
						 const size_t maxBackupFiles) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	_sinks.push_back(new FileSink(_logDir, filename, form,
								  maxFileSize, maxBackupFiles));
}

void AccessLogger::setSinkFile(const std::string &logDir, const std::string &filename,
						 const LogFormat eFormat, const size_t maxFileSize,
						 const size_t maxBackupFiles) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	_sinks.push_back(new FileSink(logDir, filename, form, maxFileSize, maxBackupFiles));
}

void AccessLogger::setSinkConsole(const LogFormat eFormat) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	_sinks.push_back(new ConsoleSink(form));
}

void AccessLogger::log(const LogMessage &msg) {
	for (std::vector<LogSink *>::iterator it = _sinks.begin();
		 it != _sinks.end(); ++it) {
		LogSink *sink = *it;
		if ((sink->getFilterMode() == EXACT &&
			 msg.level == sink->getLogLevel()) ||
			(sink->getFilterMode() == GREATER_OR_EQUAL &&
			 msg.level >= sink->getLogLevel())) {
			try {
				sink->log(msg);
			} catch (const std::exception &e) {
				std::cerr << "[ ERROR ] AccessLogger: Failed to write log: "
						  << e.what() << std::endl;
			}
		}
	}
}
