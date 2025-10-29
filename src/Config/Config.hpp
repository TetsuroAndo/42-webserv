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
	static unsigned int ioBuffersSize;
	unsigned int responseReserveSize;
	unsigned int pollTimeoutMs;

	static unsigned int cgiIoBufferSize;
	unsigned int cgiMinWorkers;
	unsigned int cgiMaxWorkers;

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
	std::string indexFile;
	std::string uploadStore;
	std::map< std::string, std::string > cgiConf;
	bool session;

	bool hasRedirect;
	int redirectCode;
	std::string redirectUrl;

	Location() : autoindex(false), session(false), hasRedirect(false), redirectCode(0) {}
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
	std::map<int, std::string> _errorPages; // map: <statusCode, URI>
	unsigned int _maxRequestBodySize;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;
	unsigned int _requestHeaderTimeoutSec;
	unsigned int _requestBodyTimeoutSec;

public:
	Config(const std::vector< Listen > &listens,
		   const std::map< std::string, Location > &locations,
		   const std::vector< AccessLog > &accessLogs,
		   const std::vector< ErrorLog > &errorLogs,
		   unsigned int maxRequestBodySize, unsigned int timeoutSec,
		   unsigned int maxEvents, unsigned int requestHeaderTimeoutSec,
		   unsigned int requestBodyTimeoutSec,
		   const std::map<int, std::string> &errorPages);
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
	const std::map<int, std::string> &getErrorPages() const;
	const std::string &getErrorPage(int code) const;

	unsigned int getMaxRequestBodySize() const;
	unsigned int getTimeoutSec() const;
	unsigned int getMaxEvents() const;
	unsigned int getRequestHeaderTimeoutSec() const;
	unsigned int getRequestBodyTimeoutSec() const;

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
