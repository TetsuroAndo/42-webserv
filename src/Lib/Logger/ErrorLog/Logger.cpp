#include "Logger.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>

Logger &Logger::getInstance() {
	static Logger instance;
	return instance;
}

void Logger::setLogDir(const std::string &logDir) {
	struct stat st;
	if (stat(logDir.c_str(), &st) != 0) {
		std::cerr << "[ WARNING ] Logger: Log directory does not exist: " +
						 logDir
				  << ", using default directory: " << _logDir << std::endl;
	} else if (!S_ISDIR(st.st_mode)) {
		std::cerr
			<< "[ WARNING ] Logger: Log path exists but is not a directory: " +
				   logDir
			<< ", using default directory: " << _logDir << std::endl;
	} else {
		_logDir = logDir;
	}
}

Logger::Logger() : _logDir(_LOG_DEFAULT_DIR), _activeLevelsMask(0) {
	struct stat st;
	if (stat(_logDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
		_logDir = _LOG_FALLBACK_DIR;
		std::cerr << "[ ERROR ] Logger: Default log directory does not exist,"
				  << " using fallback directory: " << _logDir << std::endl;
	}
}

Logger::~Logger() {
	std::map<LogLevel, std::vector<LogSink *> >::iterator it;
	std::vector<LogSink*>::iterator vecIt;
	std::vector<LogSink*> deleted_sinks;

	for (it = _sinksByLevel.begin(); it != _sinksByLevel.end(); ++it) {
		for (vecIt = it->second.begin(); vecIt != it->second.end(); ++vecIt) {
			bool found = false;
			for(size_t i = 0; i < deleted_sinks.size(); ++i) {
				if (deleted_sinks[i] == *vecIt) {
					found = true;
					break;
				}
			}
			if (!found) {
				delete *vecIt;
				deleted_sinks.push_back(*vecIt);
			}
		}
	}
}

void Logger::setSinkFile(const std::string &filename, const LogFormat eFormat,
						 const LogLevel level, const LogFilterMode mode,
						 const size_t maxFileSize,
						 const size_t maxBackupFiles) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	addSink(level, mode, new FileSink(_logDir, filename, form, maxFileSize, maxBackupFiles));
}

void Logger::setSinkFile(const std::string &logDir, const std::string &filename,
						 const LogFormat eFormat, const LogLevel level,
						 const LogFilterMode mode, const size_t maxFileSize,
						 const size_t maxBackupFiles) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	addSink(level, mode, new FileSink(logDir, filename, form, maxFileSize, maxBackupFiles));

}

void Logger::setSinkConsole(const LogFormat eFormat, const LogLevel level,
							const LogFilterMode mode) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	addSink(level, mode, new ConsoleSink(form));
}

void Logger::addSink(LogLevel level, LogFilterMode mode, LogSink* sink) {
	if (mode == GREATER_OR_EQUAL) {
		for (int i = level; i <= FATAL; ++i) {
			_sinksByLevel[static_cast<LogLevel>(i)].push_back(sink);
		}
	} else {
		_sinksByLevel[level].push_back(sink);
	}
	updateActiveLevelsMask();
}

void Logger::log(const LogMessage &msg) {
	std::map<LogLevel, std::vector<LogSink *> >::iterator it = _sinksByLevel.find(msg.level);

	if (it != _sinksByLevel.end()) {
		std::vector<LogSink *>& sinks = it->second;
		for (std::vector<LogSink *>::iterator sinkIt = sinks.begin(); sinkIt != sinks.end(); ++sinkIt) {
			try {
				(*sinkIt)->log(msg);
			} catch (const std::exception &e) {
				std::cerr << "Logger: Failed to write log: " << e.what() << std::endl;
			}
		}
	}
}

bool Logger::isLogLevelActive(const LogLevel level) const {
	return _activeLevelsMask >> level & 1;
}

void Logger::updateActiveLevelsMask() {
	_activeLevelsMask = 0;

	std::map<LogLevel, std::vector<LogSink *> >::iterator it;
	for (it = _sinksByLevel.begin(); it != _sinksByLevel.end(); ++it) {
		if (!it->second.empty()) {
			_activeLevelsMask |= (1 << it->first);
		}
	}
}
