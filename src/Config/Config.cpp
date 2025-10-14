#include "Config.hpp"
#include "../Lib/Logger/ErrorLog/Logger.hpp"
#include "../Lib/MyYAML/MyYAML.hpp"
#include "../Lib/StringOps/StringOps.hpp"
#include "ConfigParser.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

void Config::initDefaults() {
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

void Config::setup(const std::string &configFile) {
	LOG(INFO) << "Loading configuration from: " << configFile;
	const MyYAML yaml(configFile);
	const Node *serversNode = yaml.getData().getMapNode("servers");
	if (!serversNode) {
		throw std::runtime_error("Config error: missing 'servers' root node");
	}

	const std::vector<Node *> &serverList = serversNode->getSeq();
	if (serverList.empty()) {
		throw std::runtime_error("Config error: no servers configured");
	}

	Node *serverNode = serverList[0];
	if (serverNode->getKey() != "server") {
		throw std::runtime_error(
			"Config error: missing 'server' key in server list");
	}

	ConfigParser parser(this);
	parser.parseServer(serverNode);
}

Config::Config() {
	initDefaults();
	setup("config/default.yaml");
}

Config::Config(const std::string &configFile) {
	initDefaults();
	setup(configFile);
}

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

void Config::setRoot(const std::string &root, const std::string &locationKey) {
	_locations[locationKey].root = root;
}

void Config::setAutoindex(const bool autoindex,
						  const std::string &locationKey) {
	_locations[locationKey].autoindex = autoindex;
}

void Config::setIndexFile(const std::string &indexFile,
						  const std::string &locationKey) {
	_locations[locationKey].indexFile = indexFile;
}

void Config::setErrorFile(const std::string &errorFile,
						  const std::string &locationKey) {
	_locations[locationKey].errorFile = errorFile;
}

void Config::setUploadStore(const std::string &uploadStore,
							const std::string &locationKey) {
	_locations[locationKey].uploadStore = uploadStore;
}

void Config::setCgiConf(const std::string &extension,
						const std::string &interpreterPath,
						const std::string &locationKey) {
	_locations[locationKey].cgiConf[extension] = interpreterPath;
}

void Config::setIsAllowGet(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("GET");
	else
		_locations[locationKey].allowedMethods.erase("GET");
}

void Config::setIsAllowHead(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("HEAD");
	else
		_locations[locationKey].allowedMethods.erase("HEAD");
}

void Config::setIsAllowPost(const bool allow, const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("POST");
	else
		_locations[locationKey].allowedMethods.erase("POST");
}

void Config::setIsAllowDelete(const bool allow,
							  const std::string &locationKey) {
	if (allow)
		_locations[locationKey].allowedMethods.insert("DELETE");
	else
		_locations[locationKey].allowedMethods.erase("DELETE");
}

void Config::setAllowedMethods(const std::string &methods,
							   const std::string &locationKey) {
	std::set<std::string> methodSet;
	std::stringstream ss(methods);
	std::string method;
	while (ss >> method) {
		methodSet.insert(method);
	}
	_locations[locationKey].allowedMethods = methodSet;
}

void Config::setAllowedMethods(const std::set<std::string> &methods,
							   const std::string &locationKey) {
	_locations[locationKey].allowedMethods = methods;
}

void Config::setListens(const std::vector<Listen> &lists) { _listens = lists; }

void Config::setAccessLogs(const std::vector<AccessLog> &accessLogs) {
	_accessLogs = accessLogs;
}

void Config::setErrorLogs(const std::vector<ErrorLog> &errorLogs) {
	_errorLogs = errorLogs;
}

void Config::setRedirects(const std::map<std::string, Redirect> &redirects) {
	_redirects = redirects;
}

void Config::setRedirect(const Redirect &redirect,
						 const std::string &redirectKey) {
	_redirects[redirectKey] = redirect;
}

void Config::setLocations(const std::map<std::string, Location> &locations) {
	_locations = locations;
}

void Config::setLocation(const Location &location,
						 const std::string &locationKey) {
	_locations[locationKey] = location;
}

void Config::setMaxRequestBodySize(const unsigned int size) {
	_maxRequestBodySize = size;
}

void Config::setTimeoutSec(const unsigned int sec) { _timeoutSec = sec; }

void Config::setMaxEvents(const unsigned int maxEvents) {
	_maxEvents = maxEvents;
}

const std::vector<Listen> &Config::getListens() const { return _listens; }

const std::map<std::string, Redirect> &Config::getRedirects() const {
	return _redirects;
}

const Redirect &Config::getRedirect(const std::string &path) const {
	std::string bestMatchKey = "";

	for (std::map<std::string, Redirect>::const_iterator it =
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

const std::map<std::string, Location> &Config::getLocations() const {
	return _locations;
}

const Location &Config::getLocation(const std::string &path) const {
	std::string bestMatchKey = "";

	for (std::map<std::string, Location>::const_iterator it =
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
		std::map<std::string, Location>::const_iterator it =
			_locations.find(bestMatchKey);
		return it->second;
	}
	std::map<std::string, Location>::const_iterator it = _locations.find("/");
	if (it != _locations.end()) {
		return it->second;
	}
	throw std::runtime_error("Config error: default location '/' not found");
}

const std::vector<AccessLog> &Config::getAccessLogs() const {
	return _accessLogs;
}

const std::vector<ErrorLog> &Config::getErrorLogs() const { return _errorLogs; }

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
	for (std::vector<Listen>::const_iterator it = config._listens.begin();
		 it != config._listens.end(); ++it) {
		os << "    - " << it->interface << ":" << it->port << "\n";
	}

	os << "  redirects:\n";
	for (std::map<std::string, Redirect>::const_iterator it =
			 config._redirects.begin();
		 it != config._redirects.end(); ++it) {
		os << "    - from: " << it->second.fromPath
		   << ", to: " << it->second.toUrl << ", code: " << it->second.code
		   << "\n";
	}

	os << "  locations:\n";
	for (std::map<std::string, Location>::const_iterator it =
			 config._locations.begin();
		 it != config._locations.end(); ++it) {
		os << "  - path: " << it->second.path << "\n";
		os << "      root: " << it->second.root << "\n";
		os << "      allowedMethods: ";
		for (std::set<std::string>::const_iterator mit =
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
		for (std::map<std::string, std::string>::const_iterator cit =
				 it->second.cgiConf.begin();
			 cit != it->second.cgiConf.end(); ++cit) {
			os << "        " << cit->first << ": " << cit->second << "\n";
		}
	}

	os << "  accessLogs:\n";
	for (std::vector<AccessLog>::const_iterator it = config._accessLogs.begin();
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
	for (std::vector<ErrorLog>::const_iterator it = config._errorLogs.begin();
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
