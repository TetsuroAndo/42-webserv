#include "ConfigBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "ConfigParser.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

void ConfigBuilder::initDefaults() {
	_listens.clear();
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

	_defaultLocationKey = "/";

	Location defaultLoc;
	defaultLoc.path = _defaultLocationKey;
	defaultLoc.root = "./www";
	defaultLoc.uploadStore = "./www/uploads";
	defaultLoc.index = "index.html";
	defaultLoc.directoryError = "";
	defaultLoc.autoindex = true;
	defaultLoc.allowedMethods.insert("GET");
	defaultLoc.allowedMethods.insert("HEAD");
	defaultLoc.allowedMethods.insert("POST");
	defaultLoc.allowedMethods.insert("DELETE");
	defaultLoc.session = true;
	_locations["/"] = defaultLoc;

	_accessLogs.push_back(AccessLog());
	_errorLogs.push_back(ErrorLog());
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

	Node *serverNode = serverList[0];
	if (serverNode->getKey() != "server") {
		throw std::runtime_error(
			"ConfigBuilder error: missing 'server' key in server list");
	}

	ConfigParser parser(this);
	parser.parseServer(serverNode);
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
	return Config(_listens, _locations, _accessLogs, _errorLogs,
				  _maxRequestBodySize, _hasBiggestRequestBodySize,
				  _biggestRequestBodySize, _timeoutSec, _maxEvents,
				  _requestHeaderTimeoutSec, _requestBodyTimeoutSec,
				  _errorPages);
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
	_locations[newLocation.path] = newLocation;
}

void ConfigBuilder::setServerDefaultRoot(const std::string &root) {
	_locations[_defaultLocationKey].root = root;
}

void ConfigBuilder::setServerDefaultAutoindex(bool autoindex) {
	_locations[_defaultLocationKey].autoindex = autoindex;
}

void ConfigBuilder::setServerDefaultindex(const std::string &index) {
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
