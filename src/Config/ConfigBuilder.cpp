#include "ConfigBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "ConfigParser.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

void ConfigBuilder::initDefaults() {
	_servers.clear();
	resetCurrentServer();
}

void ConfigBuilder::resetCurrentServer() {
	_listens.clear();
	_serverNames.clear();
	_locations.clear();
	_accessLogs.clear();
	_errorLogs.clear();
	_errorPages.clear();

	_maxRequestBodySize = 1024 * 1024;
	_hasBiggestRequestBodySize = false;
	_biggestRequestBodySize = 0;
	_maxEvents = 1024;

	_timeoutSec = 60;
	_requestHeaderTimeoutSec = 20;
	_requestBodyTimeoutSec = 30;

	_maxRequestHeaderSize = 8192;

	_sessionTimeoutSec = 1800;

	_defaultLocationKey = "/";

	Location defaultLoc;
	defaultLoc.path = _defaultLocationKey;
	defaultLoc.root = "./www";
	defaultLoc.uploadStore = "./www/uploads";
	defaultLoc.index = "index.html";
	defaultLoc.noIndex = false;
	defaultLoc.directoryError = "";
	defaultLoc.autoindex = true;
	defaultLoc.allowedMethods.insert("GET");
	defaultLoc.allowedMethods.insert("HEAD");
	defaultLoc.allowedMethods.insert("POST");
	defaultLoc.allowedMethods.insert("DELETE");
	defaultLoc.session = true;
	defaultLoc.hasChunkedTimeoutSec = true;
	defaultLoc.chunkedTimeoutSec = 10;
	_locations["/"] = defaultLoc;

	_accessLogs.push_back(AccessLog());
	_errorLogs.push_back(ErrorLog());
}

void ConfigBuilder::pushCurrentServer() {
	ServerConfig server;
	server.listens = _listens;
	server.serverNames = _serverNames;
	server.locations = _locations;
	server.accessLogs = _accessLogs;
	server.errorLogs = _errorLogs;
	server.errorPages = _errorPages;
	server.maxRequestBodySize = _maxRequestBodySize;
	server.hasBiggestMaxRequestBodySize = _hasBiggestRequestBodySize;
	server.biggestMaxRequestBodySize = _biggestRequestBodySize;
	server.timeoutSec = _timeoutSec;
	server.maxEvents = _maxEvents;
	server.requestHeaderTimeoutSec = _requestHeaderTimeoutSec;
	server.requestBodyTimeoutSec = _requestBodyTimeoutSec;
	server.sessionTimeoutSec = _sessionTimeoutSec;
	server.maxRequestHeaderSize = _maxRequestHeaderSize;
	_servers.push_back(server);
}

void ConfigBuilder::setup(const std::string &configFile) {
	LOG(INFO) << "Loading configuration from: " << configFile;
	const MyYAML yaml(configFile);
	const Node *serversNode = yaml.getData().getMapNode("servers");
	if (!serversNode) {
		throw std::runtime_error(
			"ConfigBuilder error: missing 'servers' root node");
	}

	const std::vector< Node * > &serverList = serversNode->getSeq();
	if (serverList.empty()) {
		throw std::runtime_error("ConfigBuilder error: no servers configured");
	}

	ConfigParser parser(this);
	for (std::vector< Node * >::const_iterator it = serverList.begin();
		 it != serverList.end(); ++it) {
		Node *serverNode = *it;
		if (serverNode->getKey() != "server") {
			throw std::runtime_error(
				"ConfigBuilder error: missing 'server' key in server list");
		}

		// 各serverを処理する前に現在のserver状態をリセット
		resetCurrentServer();
		parser.parseServer(serverNode);
		// server処理完了後、現在のserverを_serversに追加
		pushCurrentServer();
	}
}

ConfigBuilder::ConfigBuilder() {
	initDefaults();
	setup("config/default.yaml");
}

ConfigBuilder::ConfigBuilder(const std::string &configFile) {
	initDefaults();
	setup(configFile);
}

ConfigBuilder::~ConfigBuilder() {}

Config ConfigBuilder::build() const {
	return Config(_servers);
}

void ConfigBuilder::setMaxRequestBodySize(const size_t size) {
	_maxRequestBodySize = size;
}

void ConfigBuilder::setBiggestRequestBodySize(bool hasValue, size_t size) {
	_hasBiggestRequestBodySize = hasValue;
	_biggestRequestBodySize = size;
}

void ConfigBuilder::setTimeoutSec(const size_t sec) { _timeoutSec = sec; }

void ConfigBuilder::setMaxEvents(const size_t maxEvents) {
	_maxEvents = maxEvents;
}

void ConfigBuilder::setListens(const std::vector< Listen > &lists) {
	_listens = lists;
}

void ConfigBuilder::setAccessLogs(const std::vector< AccessLog > &accessLogs) {
	_accessLogs = accessLogs;
}

void ConfigBuilder::setErrorLogs(const std::vector< ErrorLog > &errorLogs) {
	_errorLogs = errorLogs;
}

void ConfigBuilder::setErrorPage(int code, const std::string &uri) {
	_errorPages[code] = uri;
}

void ConfigBuilder::setLocations(
	const std::map< std::string, Location > &locations) {
	_locations = locations;
}

void ConfigBuilder::setLocation(const Location &location) {
	Location newLocation = location;
	const Location &defaultLocation = _locations[_defaultLocationKey];

	if (newLocation.root.empty()) {
		newLocation.root = defaultLocation.root;
	}
	if (newLocation.allowedMethods.empty()) {
		newLocation.allowedMethods = defaultLocation.allowedMethods;
	}
	if (newLocation.index.empty()) {
		newLocation.index = defaultLocation.index;
	}
	std::map< std::string, std::string >::const_iterator it =
		newLocation.cgiConf.begin();
	while (it != newLocation.cgiConf.end()) {
		if (StringOps::endsWith(newLocation.index, it->first)) {
			throw std::runtime_error(
				"Config error: CGI scripts cannot be used as index files (\"" +
				it->first + "\").");
		}
		++it;
	}
	it = defaultLocation.cgiConf.begin();
	while (it != defaultLocation.cgiConf.end()) {
		if (StringOps::endsWith(newLocation.index, it->first)) {
			throw std::runtime_error(
				"Config error: CGI scripts cannot be used as index files (\"" +
				it->first + "\").");
		}
		if (newLocation.cgiConf[it->first].empty()) {
			newLocation.cgiConf[it->first] = it->second;
		}
		++it;
	}
	if (newLocation.hasChunkedTimeoutSec == false) {
		newLocation.chunkedTimeoutSec = defaultLocation.chunkedTimeoutSec;
		newLocation.hasChunkedTimeoutSec = true;
	}
	_locations[newLocation.path] = newLocation;
}

void ConfigBuilder::setSessionTimeoutSec(const size_t sec) {
	_sessionTimeoutSec = sec;
}

void ConfigBuilder::setServerName(const std::string &serverName) {
	_serverNames.push_back(serverName);
}

void ConfigBuilder::setServerDefaultRoot(const std::string &root) {
	_locations[_defaultLocationKey].root = root;
}

void ConfigBuilder::setServerDefaultAutoindex(bool autoindex) {
	_locations[_defaultLocationKey].autoindex = autoindex;
}

void ConfigBuilder::setServerDefaultIndex(const std::string &index) {
	_locations[_defaultLocationKey].index = index;
}

void ConfigBuilder::setServerDefaultUploadStore(
	const std::string &uploadStore) {
	_locations[_defaultLocationKey].uploadStore = uploadStore;
}

void ConfigBuilder::setServerDefaultCgiConf(
	const std::string &extension, const std::string &interpreterPath) {
	_locations[_defaultLocationKey].cgiConf[extension] = interpreterPath;
}

void ConfigBuilder::setServerDefaultIsAllowGet(bool allow) {
	if (allow)
		_locations[_defaultLocationKey].allowedMethods.insert("GET");
	else
		_locations[_defaultLocationKey].allowedMethods.erase("GET");
}

void ConfigBuilder::setServerDefaultIsAllowHead(bool allow) {
	if (allow)
		_locations[_defaultLocationKey].allowedMethods.insert("HEAD");
	else
		_locations[_defaultLocationKey].allowedMethods.erase("HEAD");
}

void ConfigBuilder::setServerDefaultIsAllowPost(bool allow) {
	if (allow)
		_locations[_defaultLocationKey].allowedMethods.insert("POST");
	else
		_locations[_defaultLocationKey].allowedMethods.erase("POST");
}

void ConfigBuilder::setServerDefaultIsAllowDelete(bool allow) {
	if (allow)
		_locations[_defaultLocationKey].allowedMethods.insert("DELETE");
	else
		_locations[_defaultLocationKey].allowedMethods.erase("DELETE");
}

void ConfigBuilder::setServerDefaultAllowedMethods(const std::string &methods) {
	std::set< std::string > methodsSet;
	std::istringstream iss(methods);
	std::string method;
	while (iss >> method) {
		if (!method.empty()) {
			methodsSet.insert(method);
		}
	}
	_locations[_defaultLocationKey].allowedMethods = methodsSet;
}

void ConfigBuilder::setServerDefaultAllowedMethods(
	const std::set< std::string > &methods) {
	_locations[_defaultLocationKey].allowedMethods = methods;
}

void ConfigBuilder::setServerDefaultSession(bool enable) {
	_locations[_defaultLocationKey].session = enable;
}

void ConfigBuilder::setRequestHeaderTimeoutSec(const size_t sec) {
	_requestHeaderTimeoutSec = sec;
}

void ConfigBuilder::setRequestBodyTimeoutSec(const size_t sec) {
	_requestBodyTimeoutSec = sec;
}

void ConfigBuilder::setMaxRequestHeaderSize(const size_t size) {
	_maxRequestHeaderSize = size;
}

void ConfigBuilder::setServerDefaultChunkedTimeoutSec(const size_t sec) {
	_locations[_defaultLocationKey].chunkedTimeoutSec = sec;
}
