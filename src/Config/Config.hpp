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

struct Listen {
	std::string interface;
	int port;
};

struct Redirect {
	std::string fromPath;
	std::string toUrl;
	int code;
};

struct Location {
	std::string path;
	std::string root;
	std::set< std::string > allowedMethods;
	bool autoindex;
	std::string indexFile;
	std::string errorFile;
	std::string uploadStore;
	std::map< std::string, std::string > cgiConf;

	Location() : autoindex(false) {}
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
	std::vector< Listen > _listens;
	std::map< std::string, Redirect > _redirects;
	std::map< std::string, Location > _locations;
	std::vector< AccessLog > _accessLogs;
	std::vector< ErrorLog > _errorLogs;
	unsigned int _maxRequestBodySize;
	unsigned int _timeoutSec;
	unsigned int _maxEvents;
	unsigned int _ioBufferSize;
	unsigned int _maxCgiResponseSize;
	size_t _maxHeaderValueSize;
	size_t _maxHeaderKeys;
	size_t _maxHeaderValuesPerKey;
	size_t _maxResponseBodySize;
	unsigned int _sessionCleanupIntervalSec;

public:
	Config(const std::vector< Listen > &listens,
		   const std::map< std::string, Redirect > &redirects,
		   const std::map< std::string, Location > &locations,
		   const std::vector< AccessLog > &accessLogs,
		   const std::vector< ErrorLog > &errorLogs,
		   unsigned int maxRequestBodySize, unsigned int timeoutSec,
		   unsigned int maxEvents, unsigned int ioBufferSize,
		   unsigned int maxCgiResponseSize, size_t maxHeaderValueSize,
		   size_t maxHeaderKeys, size_t maxHeaderValuesPerKey,
		   size_t maxResponseBodySize, unsigned int sessionCleanupIntervalSec);
	Config(const Config &other);
	Config &operator=(const Config &other);
	~Config();

	const AppInfo &getAppInfo() const;
	const std::vector< Listen > &getListens() const;
	const std::map< std::string, Redirect > &getRedirects() const;
	const Redirect &getRedirect(const std::string &path) const;
	const std::map< std::string, Location > &getLocations() const;
	const Location &getLocation(const std::string &path) const;
	const std::vector< AccessLog > &getAccessLogs() const;
	const std::vector< ErrorLog > &getErrorLogs() const;

	unsigned int getMaxRequestBodySize() const;
	unsigned int getTimeoutSec() const;
	unsigned int getMaxEvents() const;
	unsigned int getIoBufferSize() const;
	unsigned int getMaxCgiResponseSize() const;
	size_t getMaxHeaderValueSize() const;
	size_t getMaxHeaderKeys() const;
	size_t getMaxHeaderValuesPerKey() const;
	size_t getMaxResponseBodySize() const;
	unsigned int getSessionCleanupIntervalSec() const;

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
