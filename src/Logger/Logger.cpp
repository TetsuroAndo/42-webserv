#include "Logger.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>

Logger& Logger::getInstance() {
	static Logger instance;
	return instance;
}

void Logger::setLogDir(const std::string &logDir) {
	struct stat st;
	if (stat(logDir.c_str(), &st) != 0) {
		std::cerr << "[ WARNING ] Logger: Log directory does not exist: " + logDir
				  << ", using default directory: " << _logDir << std::endl;
	} else if (!S_ISDIR(st.st_mode)) {
		std::cerr << "[ WARNING ] Logger: Log path exists but is not a directory: " + logDir
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
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		delete *it;
	}
	_sinks.clear();
}

void Logger::setSinkFile(const std::string &filename, const LogFormat eFormat,
	const LogLevel level, const LogFilterMode mode, const size_t maxFileSize, const size_t maxBackupFiles)
{
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	_sinks.push_back(new FileSink(_logDir, filename, form, level, mode, maxFileSize, maxBackupFiles));
	updateActiveLevelsMask();
}

void Logger::setSinkFile(const std::string &logDir, const std::string &filename,
	const LogFormat eFormat, const LogLevel level, const LogFilterMode mode,
	const size_t maxFileSize, const size_t maxBackupFiles) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	_sinks.push_back(new FileSink(logDir, filename, form, level, mode, maxFileSize, maxBackupFiles));
	updateActiveLevelsMask();
}

void Logger::setSinkConsole(const LogFormat eFormat, const LogLevel level, const LogFilterMode mode) {
	LogForm *form;
	if (eFormat == JSON) {
		form = new JsonForm();
	} else {
		form = new ElfForm();
	}
	_sinks.push_back(new ConsoleSink(form, level, mode));
	updateActiveLevelsMask();
}

void Logger::log(const LogMessage& msg) {
	for (std::vector<LogSink*>::iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		LogSink* sink = *it;
		if ((sink->getFilterMode() == EXACT && msg.level == sink->getLogLevel()) ||
			(sink->getFilterMode() == GREATER_OR_EQUAL && msg.level >= sink->getLogLevel()))
		{
			try {
				sink->log(msg);
			} catch (const std::exception& e) {
				std::cerr << "[ ERROR ] Logger: Failed to write log: " << e.what() << std::endl;
			}
		}
	}
}

bool Logger::isLogLevelActive(const LogLevel level) const {
	return _activeLevelsMask >> level & 1;
}

void Logger::updateActiveLevelsMask() {
	_activeLevelsMask = 0;
	for (std::vector<LogSink*>::const_iterator it = _sinks.begin(); it != _sinks.end(); ++it) {
		const LogLevel level = (*it)->getLogLevel();
		if ((*it)->getFilterMode() == GREATER_OR_EQUAL) {
			for (int i = level; i <= FATAL; ++i) {
				_activeLevelsMask |= 1 << i;
			}
		} else {
			_activeLevelsMask |= 1 << level;
		}
	}
}
