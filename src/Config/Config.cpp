#include "Config.hpp"
#include "../Lib/Logger/Log.hpp"
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

unsigned int Performance::ioBuffersSize = IO_BUFFER_SIZE;
unsigned int Performance::cgiIoBufferSize = CGI_IO_BUFFER_SIZE;
Performance::Performance()
	: responseReserveSize(RESPONSE_RESERVE_SIZE),
	  pollTimeoutMs(POLL_TIMEOUT_MS),
	  cgiMinWorkers(CGI_MIN_WORKERS),
	  cgiMaxWorkers(CGI_MAX_WORKERS) {}

// clang-format on

Config::Config(const std::vector< Listen > &listens,
			   const std::map< std::string, Redirect > &redirects,
			   const std::map< std::string, Location > &locations,
			   const std::vector< AccessLog > &accessLogs,
			   const std::vector< ErrorLog > &errorLogs,
			   unsigned int maxRequestBodySize, unsigned int timeoutSec,
			   unsigned int maxEvents)
	: _listens(listens), _redirects(redirects), _locations(locations),
	  _accessLogs(accessLogs), _errorLogs(errorLogs),
	  _maxRequestBodySize(maxRequestBodySize), _timeoutSec(timeoutSec),
	  _maxEvents(maxEvents) {}

Config::Config(const Config &other)
	: _listens(other._listens), _redirects(other._redirects),
	  _locations(other._locations), _accessLogs(other._accessLogs),
	  _errorLogs(other._errorLogs),
	  _maxRequestBodySize(other._maxRequestBodySize),
	  _timeoutSec(other._timeoutSec), _maxEvents(other._maxEvents) {}

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		_listens = other._listens;
		_redirects = other._redirects;
		_locations = other._locations;
		_accessLogs = other._accessLogs;
		_errorLogs = other._errorLogs;
		_maxRequestBodySize = other._maxRequestBodySize;
		_timeoutSec = other._timeoutSec;
		_maxEvents = other._maxEvents;
	}
	return *this;
}

Config::~Config() {}

const AppInfo &Config::getAppInfo() const { return _appInfo; }

const Performance &Config::getPerformance() const { return _performance; }

const std::vector< Listen > &Config::getListens() const { return _listens; }

const std::map< std::string, Redirect > &Config::getRedirects() const {
	return _redirects;
}

const Redirect &Config::getRedirect(const std::string &path) const {
	std::string bestMatchKey = "";

	for (std::map< std::string, Redirect >::const_iterator it =
			 _redirects.begin();
		 it != _redirects.end(); ++it) {
		const std::string &redirectPath = it->first;
		if (path.rfind(redirectPath, 0) == 0) {
			if (redirectPath.length() > bestMatchKey.length()) {
				bestMatchKey = redirectPath;
			}
		}
	}
	if (!bestMatchKey.empty()) {
		return _redirects.at(bestMatchKey);
	}
	// Return a default constructed Redirect indicating no match
	static const Redirect noMatchRedirect = {"", "", 0};
	return noMatchRedirect;
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

unsigned int Config::getMaxRequestBodySize() const {
	return _maxRequestBodySize;
}

unsigned int Config::getTimeoutSec() const { return _timeoutSec; }

unsigned int Config::getMaxEvents() const { return _maxEvents; }

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << "Config:\n";
	os << "  maxRequestBodySize: " << config._maxRequestBodySize << "\n";
	os << "  timeoutSec: " << config._timeoutSec << "\n";
	os << "  maxEvents: " << config._maxEvents << "\n";

	os << "  listens:\n";
	for (std::vector< Listen >::const_iterator it = config._listens.begin();
		 it != config._listens.end(); ++it) {
		os << "    - " << it->interface << ":" << it->port << "\n";
	}

	os << "  redirects:\n";
	for (std::map< std::string, Redirect >::const_iterator it =
			 config._redirects.begin();
		 it != config._redirects.end(); ++it) {
		os << "    - from: " << it->second.fromPath
		   << ", to: " << it->second.toUrl << ", code: " << it->second.code
		   << "\n";
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
		os << "      indexFile: " << it->second.indexFile << "\n";
		os << "      errorFile: " << it->second.errorFile << "\n";
		os << "      uploadStore: " << it->second.uploadStore << "\n";
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
