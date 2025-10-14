#include "ConfigBuilder.hpp"
#include "../Lib/Logger/Log.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "ConfigParser.hpp"
#include <sstream>

void ConfigBuilder::initDefaults() {
	_listens.clear();
	_redirects.clear();
	_locations.clear();
	_accessLogs.clear();
	_errorLogs.clear();

	_maxRequestBodySize = 1024 * 1024;
	_timeoutSec = 60;
	_maxEvents = 1024;

	Location defaultLoc;
	defaultLoc.path = "/";
	defaultLoc.root = "/tmp/www";
	defaultLoc.uploadStore = "/tmp/uploads";
	defaultLoc.indexFile = "index.html";
	defaultLoc.autoindex = true;
	defaultLoc.allowedMethods.insert("GET");
	defaultLoc.allowedMethods.insert("HEAD");
	defaultLoc.allowedMethods.insert("POST");
	defaultLoc.allowedMethods.insert("DELETE");
	_locations["/"] = defaultLoc;

	_accessLogs.push_back(AccessLog());
	_errorLogs.push_back(ErrorLog());
}

void ConfigBuilder::setup(const std::string &configFile) {
	LOG(INFO) << "Loading configuration from: " << configFile;
	const MyYAML yaml(configFile);
	const Node *serversNode = yaml.getData().getMapNode("servers");
	if (!serversNode) {
		throw std::runtime_error("ConfigBuilder error: missing 'servers' root node");
	}

	const std::vector<Node *> &serverList = serversNode->getSeq();
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

Config ConfigBuilder::build() const {
	return Config(_listens, _redirects, _locations, _accessLogs, _errorLogs,
				  _maxRequestBodySize, _timeoutSec, _maxEvents);
}

const Location &ConfigBuilder::getLocation(const std::string &key) const {
	return _locations.at(key);
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

void ConfigBuilder::setRoot(const std::string &root, const std::string &locationKey) {
	_locations[locationKey].root = root;
}

void ConfigBuilder::setAutoindex(const bool autoindex,
						  const std::string &locationKey) {
	_locations[locationKey].autoindex = autoindex;
}

void ConfigBuilder::setIndexFile(const std::string &indexFile,
						  const std::string &locationKey) {
	_locations[locationKey].indexFile = indexFile;
}

void ConfigBuilder::setErrorFile(const std::string &errorFile,
						  const std::string &locationKey) {
	_locations[locationKey].errorFile = errorFile;
}

void ConfigBuilder::setUploadStore(const std::string &uploadStore,
							const std::string &locationKey) {
	_locations[locationKey].uploadStore = uploadStore;
}

void ConfigBuilder::setCgiConf(const std::string &extension,
						const std::string &interpreterPath,
						const std::string &locationKey) {
	_locations[locationKey].cgiConf[extension] = interpreterPath;
}

void ConfigBuilder::setIsAllowGet(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("GET");
	else
		_locations[locationKey].allowedMethods.erase("GET");
}

void ConfigBuilder::setIsAllowHead(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("HEAD");
	else
		_locations[locationKey].allowedMethods.erase("HEAD");
}

void ConfigBuilder::setIsAllowPost(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("POST");
	else
		_locations[locationKey].allowedMethods.erase("POST");
}

void ConfigBuilder::setIsAllowDelete(const bool allow,
							  const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("DELETE");
	else
		_locations[locationKey].allowedMethods.erase("DELETE");
}

void ConfigBuilder::setAllowedMethods(const std::string &methods,
							   const std::string &locationKey) {
	std::set<std::string> methodSet;
	std::stringstream ss(methods);
	std::string method;
	while (ss >> method) {
		methodSet.insert(method);
	}
	_locations[locationKey].allowedMethods = methodSet;
}

void ConfigBuilder::setAllowedMethods(const std::set<std::string> &methods,
							   const std::string &locationKey) {
	_locations[locationKey].allowedMethods = methods;
}

void ConfigBuilder::setListens(const std::vector<Listen> &lists) { _listens = lists; }

void ConfigBuilder::setAccessLogs(const std::vector<AccessLog> &accessLogs) {
	_accessLogs = accessLogs;
}

void ConfigBuilder::setErrorLogs(const std::vector<ErrorLog> &errorLogs) {
	_errorLogs = errorLogs;
}

void ConfigBuilder::setRedirects(const std::map<std::string, Redirect> &redirects) {
	_redirects = redirects;
}

void ConfigBuilder::setRedirect(const Redirect &redirect,
						 const std::string &redirectKey) {
	_redirects[redirectKey] = redirect;
}

void ConfigBuilder::setLocations(const std::map<std::string, Location> &locations) {
	_locations = locations;
}

void ConfigBuilder::setLocation(const Location &location,
						 const std::string &locationKey) {
	_locations[locationKey] = location;
}

void ConfigBuilder::setMaxRequestBodySize(const unsigned int size) {
	_maxRequestBodySize = size;
}

void ConfigBuilder::setTimeoutSec(const unsigned int sec) { _timeoutSec = sec; }

void ConfigBuilder::setMaxEvents(const unsigned int maxEvents) {
	_maxEvents = maxEvents;
}
