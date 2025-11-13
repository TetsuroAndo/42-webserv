#include "Config.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "Info/App.hpp"
#include "PerformanceConfig.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

// clang-format off

AppInfo::AppInfo()
	: softwareName(SOFTWARE_NAME),
	  softwareVersion(VERSION),
	  httpServerName(SERVER_NAME),
	  httpProtocolVersion(HTTP_VERSION),
	  cgiVersion(CGI_VERSION) {}

size_t Performance::ioBuffersSize = IO_BUFFER_SIZE;
size_t Performance::cgiIoBufferSize = CGI_IO_BUFFER_SIZE;
Performance::Performance()
	: responseReserveSize(RESPONSE_RESERVE_SIZE),
	  pollTimeoutMs(POLL_TIMEOUT_MS),
	  cgiMinWorkers(CGI_MIN_WORKERS),
	  cgiMaxWorkers(CGI_MAX_WORKERS) {}

// clang-format on

Config::Config(const std::vector< Listen > &listens,
			   const std::map< std::string, Location > &locations,
			   const std::vector< AccessLog > &accessLogs,
			   const std::vector< ErrorLog > &errorLogs,
			   size_t maxRequestBodySize, bool hasBiggestMaxRequestBodySize,
			   size_t biggestMaxRequestBodySize, size_t timeoutSec,
			   size_t maxEvents, size_t requestHeaderTimeoutSec,
			   size_t requestBodyTimeoutSec,
			   const std::map< int, std::string > &errorPages)
	: _listens(listens), _locations(locations), _accessLogs(accessLogs),
	  _errorLogs(errorLogs), _errorPages(errorPages),
	  _maxRequestBodySize(maxRequestBodySize),
	  _hasBiggestMaxRequestBodySize(hasBiggestMaxRequestBodySize),
	  _biggestMaxRequestBodySize(biggestMaxRequestBodySize),
	  _timeoutSec(timeoutSec), _maxEvents(maxEvents),
	  _requestHeaderTimeoutSec(requestHeaderTimeoutSec),
	  _requestBodyTimeoutSec(requestBodyTimeoutSec) {}

Config::Config(const Config &other)
	: _listens(other._listens), _locations(other._locations),
	  _accessLogs(other._accessLogs), _errorLogs(other._errorLogs),
	  _errorPages(other._errorPages),
	  _maxRequestBodySize(other._maxRequestBodySize),
	  _hasBiggestMaxRequestBodySize(other._hasBiggestMaxRequestBodySize),
	  _biggestMaxRequestBodySize(other._biggestMaxRequestBodySize),
	  _timeoutSec(other._timeoutSec), _maxEvents(other._maxEvents),
	  _requestHeaderTimeoutSec(other._requestHeaderTimeoutSec),
	  _requestBodyTimeoutSec(other._requestBodyTimeoutSec) {}

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		_listens = other._listens;
		_locations = other._locations;
		_accessLogs = other._accessLogs;
		_errorLogs = other._errorLogs;
		_errorPages = other._errorPages;
		_maxRequestBodySize = other._maxRequestBodySize;
		_hasBiggestMaxRequestBodySize = other._hasBiggestMaxRequestBodySize;
		_biggestMaxRequestBodySize = other._biggestMaxRequestBodySize;
		_timeoutSec = other._timeoutSec;
		_maxEvents = other._maxEvents;
		_requestHeaderTimeoutSec = other._requestHeaderTimeoutSec;
		_requestBodyTimeoutSec = other._requestBodyTimeoutSec;
	}
	return *this;
}

Config::~Config() {}

const AppInfo &Config::getAppInfo() const { return _appInfo; }

const Performance &Config::getPerformance() const { return _performance; }

const std::vector< Listen > &Config::getListens() const { return _listens; }

const std::map< int, std::string > &Config::getErrorPages() const {
	return _errorPages;
}

const std::string &Config::getErrorPage(int code) const {
	std::map< int, std::string >::const_iterator it = _errorPages.find(code);
	if (it != _errorPages.end()) {
		return it->second;
	}
	static const std::string empty;
	return empty;
}

const std::map< std::string, Location > &Config::getLocations() const {
	return _locations;
}

const Location &Config::getLocation(const std::string &path) const {
	std::string bestMatchKey = "";

	for (std::map< std::string, Location >::const_iterator it =
			 _locations.begin();
		 it != _locations.end(); ++it) {
		const std::string &locPath = it->first;
		if (path.rfind(locPath, 0) == 0) {
			if (locPath.length() > bestMatchKey.length()) {
				bestMatchKey = locPath;
			}
		}
	}
	if (!bestMatchKey.empty()) {
		std::map< std::string, Location >::const_iterator it =
			_locations.find(bestMatchKey);
		return it->second;
	}
	std::map< std::string, Location >::const_iterator it = _locations.find("/");
	if (it != _locations.end()) {
		return it->second;
	}
	throw std::runtime_error("Config error: default location '/' not found");
}

const std::vector< AccessLog > &Config::getAccessLogs() const {
	return _accessLogs;
}

const std::vector< ErrorLog > &Config::getErrorLogs() const {
	return _errorLogs;
}

size_t Config::getMaxRequestBodySize() const { return _maxRequestBodySize; }

bool Config::hasBiggestMaxRequestBodySize() const {
	return _hasBiggestMaxRequestBodySize;
}

size_t Config::getBiggestMaxRequestBodySize() const {
	return _biggestMaxRequestBodySize;
}

size_t Config::getTimeoutSec() const { return _timeoutSec; }

size_t Config::getMaxEvents() const { return _maxEvents; }

size_t Config::getRequestHeaderTimeoutSec() const {
	return _requestHeaderTimeoutSec;
}

size_t Config::getRequestBodyTimeoutSec() const {
	return _requestBodyTimeoutSec;
}

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << "Config:\n";
	os << "  maxRequestBodySize: " << config._maxRequestBodySize << "\n";
	os << "  timeoutSec: " << config._timeoutSec << "\n";
	os << "  requestHeaderTimeoutSec: " << config._requestHeaderTimeoutSec
	   << "\n";
	os << "  requestBodyTimeoutSec: " << config._requestBodyTimeoutSec << "\n";
	os << "  maxEvents: " << config._maxEvents << "\n";

	os << "  listens:\n";
	for (std::vector< Listen >::const_iterator it = config._listens.begin();
		 it != config._listens.end(); ++it) {
		os << "    - " << it->interface << ":" << it->port << "\n";
	}

	os << "  error_pages:\n";
	for (std::map< int, std::string >::const_iterator it =
			 config._errorPages.begin();
		 it != config._errorPages.end(); ++it) {
		os << "    " << it->first << ": " << it->second << "\n";
	}

	os << "  locations:\n";
	for (std::map< std::string, Location >::const_iterator it =
			 config._locations.begin();
		 it != config._locations.end(); ++it) {
		os << "  - path: " << it->second.path << "\n";
		os << "      root: " << it->second.root << "\n";
		os << "      allowedMethods: ";
		for (std::set< std::string >::const_iterator mit =
				 it->second.allowedMethods.begin();
			 mit != it->second.allowedMethods.end(); ++mit) {
			os << *mit << " ";
		}
		os << "\n";
		os << "      autoindex: " << (it->second.autoindex ? "on" : "off")
		   << "\n";
		os << "      index: " << it->second.index << "\n";
		os << "      directoryError: " << it->second.directoryError << "\n";
		os << "      uploadStore: " << it->second.uploadStore << "\n";
		os << "      maxRequestBodySize: "
		   << (it->second.hasMaxRequestBodySize
				   ? StringOps::toString(it->second.maxRequestBodySize)
				   : "default")
		   << "\n";
		os << "      cgiConf:\n";
		for (std::map< std::string, std::string >::const_iterator cit =
				 it->second.cgiConf.begin();
			 cit != it->second.cgiConf.end(); ++cit) {
			os << "        " << cit->first << ": " << cit->second << "\n";
		}
	}

	os << "  accessLogs:\n";
	for (std::vector< AccessLog >::const_iterator it =
			 config._accessLogs.begin();
		 it != config._accessLogs.end(); ++it) {
		os << "    - isDisable: " << (it->isDisable ? "true" : "false") << "\n";
		os << "      sink: " << (it->sink == File ? "file" : "console") << "\n";
		os << "      filename: " << it->filename << "\n";
		os << "      logDir: " << it->logDir << "\n";
		os << "      format: " << (it->format == JSON ? "json" : "elf") << "\n";
		os << "      maxFileSize: " << it->maxFileSize << "\n";
		os << "      maxBackupFiles: " << it->maxBackupFiles << "\n";
	}

	os << "  errorLogs:\n";
	for (std::vector< ErrorLog >::const_iterator it = config._errorLogs.begin();
		 it != config._errorLogs.end(); ++it) {
		os << "    - isDisable: " << (it->isDisable ? "true" : "false") << "\n";
		os << "      sink: " << (it->sink == File ? "FILE" : "CONSOLE") << "\n";
		os << "      filename: " << it->filename << "\n";
		os << "      logDir: " << it->logDir << "\n";
		os << "      format: " << (it->format == JSON ? "JSON" : "ELF") << "\n";
		os << "      level: ";
		switch (it->level) {
		case DEBUG:
			os << "DEBUG";
			break;
		case INFO:
			os << "INFO";
			break;
		case WARNING:
			os << "WARNING";
			break;
		case ERROR:
			os << "ERROR";
			break;
		case FATAL:
			os << "FATAL";
			break;
		}
		os << "\n";
		os << "      filterMode: "
		   << (it->filterMode == GREATER_OR_EQUAL ? "GREATER_OR_EQUAL"
												  : "EXACT")
		   << "\n";
		os << "      maxFileSize: " << it->maxFileSize << "\n";
		os << "      maxBackupFiles: " << it->maxBackupFiles << "\n";
	}
	return os;
}
