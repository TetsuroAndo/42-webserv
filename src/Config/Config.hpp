#pragma once

#include "../Lib/Logger/ErrorLog/LogStructure.hpp"
#include "../Lib/Logger/LogType.hpp"
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

class Node;

struct AppInfo {
	std::string softwareName;
	std::string softwareVersion;

	std::string httpServerName;
	std::string httpProtocolVersion;

	std::string cgiVersion;

	AppInfo();
};

struct Performance {
	static size_t ioBuffersSize;
	size_t responseReserveSize;
	size_t pollTimeoutMs;

	static size_t cgiIoBufferSize;
	size_t cgiMinWorkers;
	size_t cgiMaxWorkers;

	Performance();
};

struct Listen {
	std::string interface;
	int port;
};

struct Location {
	std::string path;
	std::string root;
	std::set< std::string > allowedMethods;
	bool autoindex;
	bool hasMaxRequestBodySize;
	size_t maxRequestBodySize;
	std::string index;
	std::string directoryError;
	std::string uploadStore;
	std::map< std::string, std::string > cgiConf;
	bool session;

	bool hasRedirect;
	int redirectCode;
	std::string redirectUrl;

	Location()
		: autoindex(false), hasMaxRequestBodySize(false), maxRequestBodySize(0),
		  session(false), hasRedirect(false), redirectCode(0) {}
};

struct AccessLog {
	bool isDisable;
	LogSinkType sink;
	std::string filename;
	std::string logDir;
	LogFormat format;
	size_t maxFileSize;
	size_t maxBackupFiles;

	AccessLog()
		: isDisable(false), sink(File), filename("access.log"),
		  logDir("./logs"), format(ELF), maxFileSize(10 * 1024 * 1024),
		  maxBackupFiles(5) {}
};

struct ErrorLog {
	bool isDisable;
	LogSinkType sink;
	std::string filename;
	std::string logDir;
	LogFormat format;
	LogLevel level;
	LogFilterMode filterMode;
	size_t maxFileSize;
	size_t maxBackupFiles;

	ErrorLog()
		: isDisable(false), sink(File), filename("error.log"), logDir("./logs"),
		  format(ELF), level(WARNING), filterMode(GREATER_OR_EQUAL),
		  maxFileSize(10 * 1024 * 1024), maxBackupFiles(5) {}
};

class Config {
private:
	AppInfo _appInfo;
	Performance _performance;
	std::vector< Listen > _listens;
	std::map< std::string, Location > _locations;
	std::vector< AccessLog > _accessLogs;
	std::vector< ErrorLog > _errorLogs;
	std::map< int, std::string > _errorPages; // map: <statusCode, URI>
	size_t _maxRequestBodySize;
	bool _hasBiggestMaxRequestBodySize;
	size_t _biggestMaxRequestBodySize;
	size_t _timeoutSec;
	size_t _maxEvents;
	size_t _requestHeaderTimeoutSec;
	size_t _requestBodyTimeoutSec;

public:
	Config(const std::vector< Listen > &listens,
		   const std::map< std::string, Location > &locations,
		   const std::vector< AccessLog > &accessLogs,
		   const std::vector< ErrorLog > &errorLogs, size_t maxRequestBodySize,
		   bool hasBiggestMaxRequestBodySize, size_t biggestMaxRequestBodySize,
		   size_t timeoutSec, size_t maxEvents, size_t requestHeaderTimeoutSec,
		   size_t requestBodyTimeoutSec,
		   const std::map< int, std::string > &errorPages);
	Config(const Config &other);
	Config &operator=(const Config &other);
	~Config();

	const AppInfo &getAppInfo() const;
	const Performance &getPerformance() const;
	const std::vector< Listen > &getListens() const;
	const std::map< std::string, Location > &getLocations() const;
	const Location &getLocation(const std::string &path) const;
	const std::vector< AccessLog > &getAccessLogs() const;
	const std::vector< ErrorLog > &getErrorLogs() const;

	// error_pages
	const std::map< int, std::string > &getErrorPages() const;
	const std::string &getErrorPage(int code) const;

	size_t getMaxRequestBodySize() const;
	bool hasBiggestMaxRequestBodySize() const;
	size_t getBiggestMaxRequestBodySize() const;
	size_t getTimeoutSec() const;
	size_t getMaxEvents() const;
	size_t getRequestHeaderTimeoutSec() const;
	size_t getRequestBodyTimeoutSec() const;

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
