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
Config::Config(const std::vector< ServerConfig > &servers) : _servers(servers) {}

Config::Config(const Config &other) : _servers(other._servers) {}

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		_servers = other._servers;
	}
	return *this;
}

Config::~Config() {}

const AppInfo &Config::getAppInfo() const { return _appInfo; }

const Performance &Config::getPerformance() const { return _performance; }

const std::vector< ServerConfig > &Config::getServers() const {
	return _servers;
}

std::vector< Listen > Config::getAllListens() const {
	std::vector< Listen > allListens;
	for (std::vector< ServerConfig >::const_iterator it = _servers.begin();
		 it != _servers.end(); ++it) {
		for (std::vector< Listen >::const_iterator lit = it->listens.begin();
			 lit != it->listens.end(); ++lit) {
			allListens.push_back(*lit);
		}
	}
	return allListens;
}

const ServerConfig &Config::selectServer(const std::string &hostName,
										  int port) const {
	if (_servers.empty()) {
		throw std::runtime_error("Config error: no servers configured");
	}

	// serverNameとportで完全一致するserverを探す
	for (std::vector< ServerConfig >::const_iterator it = _servers.begin();
		 it != _servers.end(); ++it) {
		const ServerConfig &server = *it;
		// portが一致するかチェック
		bool portMatches = false;
		for (std::vector< Listen >::const_iterator lit = server.listens.begin();
			 lit != server.listens.end(); ++lit) {
			if (lit->port == port) {
				portMatches = true;
				break;
			}
		}
		if (!portMatches) {
			continue;
		}

		// serverNameが一致するかチェック
		if (!hostName.empty()) {
			for (std::vector< std::string >::const_iterator sit =
					 server.serverNames.begin();
				 sit != server.serverNames.end(); ++sit) {
				if (*sit == hostName) {
					return server;
				}
			}
		}
	}

	// serverNameが空（デフォルトserver）でportが一致するserverを探す
	for (std::vector< ServerConfig >::const_iterator it = _servers.begin();
		 it != _servers.end(); ++it) {
		const ServerConfig &server = *it;
		if (server.serverNames.empty()) {
			for (std::vector< Listen >::const_iterator lit =
					 server.listens.begin();
				 lit != server.listens.end(); ++lit) {
				if (lit->port == port) {
					return server;
				}
			}
		}
	}

	return _servers[0];
}

const ServerConfig &Config::getServerConfig(const std::string &hostName,
											  int port) const {
	return selectServer(hostName, port);
}

const std::map< std::string, Location > &
Config::getLocations(const std::string &hostName, int port) const {
	return selectServer(hostName, port).locations;
}

const Location &Config::getLocation(const std::string &hostName, int port,
									 const std::string &path) const {
	const std::map< std::string, Location > &locations =
		selectServer(hostName, port).locations;
	std::string bestMatchKey = "";

	for (std::map< std::string, Location >::const_iterator it =
			 locations.begin();
		 it != locations.end(); ++it) {
		const std::string &locPath = it->first;
		if (path.rfind(locPath, 0) == 0) {
			if (locPath.length() > bestMatchKey.length()) {
				bestMatchKey = locPath;
			}
		}
	}
	if (!bestMatchKey.empty()) {
		std::map< std::string, Location >::const_iterator it =
			locations.find(bestMatchKey);
		return it->second;
	}
	std::map< std::string, Location >::const_iterator it = locations.find("/");
	if (it != locations.end()) {
		return it->second;
	}
	throw std::runtime_error("Config error: default location '/' not found");
}

const std::vector< AccessLog > &
Config::getAccessLogs(const std::string &hostName, int port) const {
	return selectServer(hostName, port).accessLogs;
}

const std::vector< ErrorLog > &
Config::getErrorLogs(const std::string &hostName, int port) const {
	return selectServer(hostName, port).errorLogs;
}

const std::map< int, std::string > &
Config::getErrorPages(const std::string &hostName, int port) const {
	return selectServer(hostName, port).errorPages;
}

const std::string &Config::getErrorPage(const std::string &hostName, int port,
										 int code) const {
	const std::map< int, std::string > &errorPages =
		selectServer(hostName, port).errorPages;
	std::map< int, std::string >::const_iterator it = errorPages.find(code);
	if (it != errorPages.end()) {
		return it->second;
	}
	static const std::string empty;
	return empty;
}

size_t Config::getMaxRequestBodySize(const std::string &hostName,
									   int port) const {
	return selectServer(hostName, port).maxRequestBodySize;
}

size_t Config::getTimeoutSec(const std::string &hostName, int port) const {
	return selectServer(hostName, port).timeoutSec;
}

size_t Config::getRequestHeaderTimeoutSec(const std::string &hostName,
										   int port) const {
	return selectServer(hostName, port).requestHeaderTimeoutSec;
}

size_t Config::getRequestBodyTimeoutSec(const std::string &hostName,
										  int port) const {
	return selectServer(hostName, port).requestBodyTimeoutSec;
}

size_t Config::getSessionTimeoutSec(const std::string &hostName,
									 int port) const {
	return selectServer(hostName, port).sessionTimeoutSec;
}

size_t Config::getMaxRequestHeaderSize(const std::string &hostName,
										int port) const {
	return selectServer(hostName, port).maxRequestHeaderSize;
}

size_t Config::getMaxEvents() const {
	if (_servers.empty()) {
		throw std::runtime_error("Config error: no servers configured");
	}
	// 最大値を返す（すべてのserverで同じ値であることを期待）
	return _servers[0].maxEvents;
}

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << "Config:\n";
	os << "  servers count: " << config._servers.size() << "\n";
	for (size_t i = 0; i < config._servers.size(); ++i) {
		const ServerConfig &server = config._servers[i];
		os << "  server[" << i << "]:\n";
		os << "    serverNames:\n";
		for (std::vector< std::string >::const_iterator it =
				 server.serverNames.begin();
			 it != server.serverNames.end(); ++it) {
			os << "      - " << *it << "\n";
		}
		os << "    listens:\n";
		for (std::vector< Listen >::const_iterator it = server.listens.begin();
			 it != server.listens.end(); ++it) {
			os << "      - " << it->interface << ":" << it->port << "\n";
		}
		os << "    maxRequestBodySize: " << server.maxRequestBodySize << "\n";
		os << "    timeoutSec: " << server.timeoutSec << "\n";
		os << "    requestHeaderTimeoutSec: " << server.requestHeaderTimeoutSec
		   << "\n";
		os << "    requestBodyTimeoutSec: " << server.requestBodyTimeoutSec
		   << "\n";
		os << "    maxEvents: " << server.maxEvents << "\n";
		os << "    sessionTimeoutSec: " << server.sessionTimeoutSec << "\n";
		os << "    maxRequestHeaderSize: " << server.maxRequestHeaderSize
		   << "\n";

		os << "    error_pages:\n";
		for (std::map< int, std::string >::const_iterator it =
				 server.errorPages.begin();
			 it != server.errorPages.end(); ++it) {
			os << "      " << it->first << ": " << it->second << "\n";
		}

		os << "    locations:\n";
		for (std::map< std::string, Location >::const_iterator it =
				 server.locations.begin();
			 it != server.locations.end(); ++it) {
			os << "    - path: " << it->second.path << "\n";
			os << "        root: " << it->second.root << "\n";
			os << "        allowedMethods: ";
			for (std::set< std::string >::const_iterator mit =
					 it->second.allowedMethods.begin();
				 mit != it->second.allowedMethods.end(); ++mit) {
				os << *mit << " ";
			}
			os << "\n";
			os << "        autoindex: "
			   << (it->second.autoindex ? "on" : "off") << "\n";
			if (it->second.noIndex == false) {
				os << "        index: " << it->second.index << "\n";
			}
			os << "        noIndex: " << (it->second.noIndex ? "true" : "false")
			   << "\n";
			os << "        directoryError: " << it->second.directoryError
			   << "\n";
			os << "        uploadStore: " << it->second.uploadStore << "\n";
			os << "        maxRequestBodySize: "
			   << (it->second.hasMaxRequestBodySize
					   ? StringOps::toString(it->second.maxRequestBodySize)
					   : "default")
			   << "\n";
			os << "        chunkedTimeoutSec: "
			   << (it->second.hasChunkedTimeoutSec) << "\n";
			os << "        cgiConf:\n";
			for (std::map< std::string, std::string >::const_iterator cit =
					 it->second.cgiConf.begin();
				 cit != it->second.cgiConf.end(); ++cit) {
				os << "          " << cit->first << ": " << cit->second
				   << "\n";
			}
		}

		os << "    accessLogs:\n";
		for (std::vector< AccessLog >::const_iterator it =
				 server.accessLogs.begin();
			 it != server.accessLogs.end(); ++it) {
			os << "      - isDisable: " << (it->isDisable ? "true" : "false")
			   << "\n";
			os << "        sink: " << (it->sink == File ? "file" : "console")
			   << "\n";
			os << "        filename: " << it->filename << "\n";
			os << "        logDir: " << it->logDir << "\n";
			os << "        format: " << (it->format == JSON ? "json" : "elf")
			   << "\n";
			os << "        maxFileSize: " << it->maxFileSize << "\n";
			os << "        maxBackupFiles: " << it->maxBackupFiles << "\n";
		}

		os << "    errorLogs:\n";
		for (std::vector< ErrorLog >::const_iterator it =
				 server.errorLogs.begin();
			 it != server.errorLogs.end(); ++it) {
			os << "      - isDisable: " << (it->isDisable ? "true" : "false")
			   << "\n";
			os << "        sink: " << (it->sink == File ? "FILE" : "CONSOLE")
			   << "\n";
			os << "        filename: " << it->filename << "\n";
			os << "        logDir: " << it->logDir << "\n";
			os << "        format: " << (it->format == JSON ? "JSON" : "ELF")
			   << "\n";
			os << "        level: ";
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
			os << "        filterMode: "
			   << (it->filterMode == GREATER_OR_EQUAL ? "GREATER_OR_EQUAL"
													  : "EXACT")
			   << "\n";
			os << "        maxFileSize: " << it->maxFileSize << "\n";
			os << "        maxBackupFiles: " << it->maxBackupFiles << "\n";
		}
	}
	return os;
}
