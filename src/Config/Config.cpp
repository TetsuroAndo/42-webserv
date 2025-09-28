#include "Config.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

/* ********************* Private Setters ********************* */
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

/* ********************* Orthodox Canonical Form ********************* */
Config::Config() {
	_listens.clear();
	_redirects.clear();
	_locations.clear();
	setup();
}

Config::Config(const std::string &configFile) { setup(configFile); }

Config::Config(const Config &other)
	: _listens(other._listens), _redirects(other._redirects),
	  _locations(other._locations),
	  _maxRequestBodySize(other._maxRequestBodySize),
	  _timeoutSec(other._timeoutSec), _maxEvents(other._maxEvents),
	  _isShowDirectoryListPage(false) {}

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		_listens = other._listens;
		_redirects = other._redirects;
		_locations = other._locations;
		_maxRequestBodySize = other._maxRequestBodySize;
		_timeoutSec = other._timeoutSec;
		_maxEvents = other._maxEvents;
	}
	return *this;
}

Config::~Config() {}

/* ********************* Setup method ********************* */
void Config::setup(const std::string &configFile) {
	(void)configFile; // TODO: Implement actual file parsing

	// --- Listens ---
	Listen l1;
	l1.interface = "0.0.0.0";
	l1.port = 8080;
	_listens.push_back(l1);

	Listen l2;
	l2.interface = "127.0.0.1";
	l2.port = 3000;
	_listens.push_back(l2);

	// --- Redirects ---
	Redirect r1;
	r1.fromPath = "/old";
	r1.toUrl = "/new";
	r1.code = 301;
	_redirects[r1.fromPath] = r1;

	Redirect r2;
	r2.fromPath = "/";
	r2.toUrl = "/tmp/www/index.html";
	r2.code = 302;
	_redirects[r2.fromPath] = r2;

	// --- Locations ---
	Location defaultLoc;
	defaultLoc.path = "/";
	defaultLoc.root = "/tmp/www";
	defaultLoc.errorFile = "/tmp/www/error.html";
	defaultLoc.uploadStore = "upload";
	defaultLoc.indexFile = "/tmp/www/index.html";
	defaultLoc.autoindex = true;
	defaultLoc.allowedMethods.insert("GET");
	defaultLoc.allowedMethods.insert("HEAD");
	defaultLoc.allowedMethods.insert("POST");
	_locations[defaultLoc.path] = defaultLoc;
	setIsAllowPost(true, defaultLoc.uploadStore);
	setIsAllowHead(true, defaultLoc.path);
	setIsAllowDelete(true);

	Location uploadsLoc = _locations[defaultLoc.path];
	uploadsLoc.path = "/uploads";
	_locations[uploadsLoc.path] = uploadsLoc;
	setRoot("/tmp/uploads", uploadsLoc.path);

	_maxRequestBodySize = 1024 * 1024 * 1024; // 1MB
	_timeoutSec = 60;
	_maxEvents = 1024;
}

/* ********************* Public Getters ********************* */
const std::vector<Listen> &Config::getListens() const { return _listens; }

const std::map<std::string, Redirect> &Config::getRedirects() const {
	return _redirects;
}

const Redirect &Config::getRedirect(const std::string &path) const {
	return _redirects.at(path);
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
		const std::map<std::string, Location>::const_iterator it =
			_locations.find(bestMatchKey);
		return it->second;
	}
	const std::map<std::string, Location>::const_iterator it =
		_locations.find("/");
	if (it != _locations.end()) {
		return it->second;
	}
	throw std::runtime_error(
		"Default location '/' not found in configuration.");
}

unsigned int Config::getMaxRequestBodySize() const {
	return _maxRequestBodySize;
}

unsigned int Config::getTimeoutSec() const { return _timeoutSec; }
unsigned int Config::getMaxEvents() const { return _maxEvents; }

/* ********************* Friend Stream Operator ********************* */
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
	os << "  isShowDirectoryListPage: " << config._isShowDirectoryListPage
	   << "\n";
	os << "  whenRequestedDirectory: " << config._whenRequestedDirectory
	   << "\n";
	os << "  saveFileDirectory: " << config._saveFileDirectory << "\n";
	os << "  timeoutSec: " << config._timeoutSec << "\n";
	os << "  maxEvents: " << config._maxEvents << "\n";
	return os;
}
