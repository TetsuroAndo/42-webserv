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
	bool hasChunkedTimeoutSec;
	size_t chunkedTimeoutSec;
	std::string index;
	bool noIndex;
	std::string directoryError;
	std::string uploadStore;
	std::map< std::string, std::string > cgiConf;
	bool session;

	bool hasRedirect;
	int redirectCode;
	std::string redirectUrl;

	Location()
		: autoindex(false), hasMaxRequestBodySize(false), maxRequestBodySize(0),
		  hasChunkedTimeoutSec(false), chunkedTimeoutSec(0), noIndex(false),
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

struct ServerConfig {
	std::vector< Listen > listens;
	std::vector< std::string > serverNames;
	std::map< std::string, Location > locations;
	std::vector< AccessLog > accessLogs;
	std::vector< ErrorLog > errorLogs;
	std::map< int, std::string > errorPages;
	size_t maxRequestBodySize;
	bool hasBiggestMaxRequestBodySize;
	size_t biggestMaxRequestBodySize;
	size_t timeoutSec;
	size_t maxEvents;
	size_t requestHeaderTimeoutSec;
	size_t requestBodyTimeoutSec;
	size_t sessionTimeoutSec;
	size_t maxRequestHeaderSize;
};

class Config {
private:
	AppInfo _appInfo;
	Performance _performance;
	std::vector< ServerConfig > _servers;

	// Host名とポートに基づいて適切なserverを選択
	const ServerConfig &selectServer(const std::string &hostName,
									  int port) const;

public:
	Config(const std::vector< ServerConfig > &servers);
	Config(const Config &other);
	Config &operator=(const Config &other);
	~Config();

	const AppInfo &getAppInfo() const;
	const Performance &getPerformance() const;
	const std::vector< ServerConfig > &getServers() const;

	// すべてのserverのlistensを取得（Server初期化用）
	std::vector< Listen > getAllListens() const;

	// Host名とポートに基づいてserverを選択して各種設定を取得
	const ServerConfig &getServerConfig(const std::string &hostName,
										 int port) const;
	const std::map< std::string, Location > &
	getLocations(const std::string &hostName, int port) const;
	const Location &getLocation(const std::string &hostName, int port,
								 const std::string &path) const;
	const std::vector< AccessLog > &
	getAccessLogs(const std::string &hostName, int port) const;
	const std::vector< ErrorLog > &
	getErrorLogs(const std::string &hostName, int port) const;
	const std::map< int, std::string > &
	getErrorPages(const std::string &hostName, int port) const;
	const std::string &getErrorPage(const std::string &hostName, int port,
									 int code) const;
	size_t getMaxRequestBodySize(const std::string &hostName, int port) const;
	size_t getTimeoutSec(const std::string &hostName, int port) const;
	size_t getRequestHeaderTimeoutSec(const std::string &hostName,
									   int port) const;
	size_t getRequestBodyTimeoutSec(const std::string &hostName,
									 int port) const;
	size_t getSessionTimeoutSec(const std::string &hostName, int port) const;
	size_t getMaxRequestHeaderSize(const std::string &hostName,
									int port) const;
	size_t getMaxEvents() const; // グローバル設定として扱う（最初のserverから取得）

	friend std::ostream &operator<<(std::ostream &os, const Config &config);
};
