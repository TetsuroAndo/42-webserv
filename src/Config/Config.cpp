#include "Config.hpp"
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <ostream>
#include <string>

void Config::setListens(const std::vector<Listen> &lists) {
	this->_listens = lists;
}

void Config::setLocations(const std::vector<Location> &locations) {
	this->_locations = locations;
}

void Config::setMaxRequestBodySize(unsigned int size) {
	this->_maxRequestBodySize = size;
}

void Config::setTimeoutSec(unsigned int sec) {
	this->_timeoutSec = sec;
}

void Config::setMaxEvents(unsigned int maxEvents) {
	this->_maxEvents = maxEvents;
}

Config::Config() { setup(); }

Config::Config(std::string configFile) {
	setup(configFile);
}

Config::~Config() {}

Config::Config(const Config &other)
	: _listens(other._listens), _locations(other._locations),
	  _maxRequestBodySize(other._maxRequestBodySize),
	  _timeoutSec(other._timeoutSec), _maxEvents(other._maxEvents) {}

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		_listens = other._listens;
		_locations = other._locations;
		_maxRequestBodySize = other._maxRequestBodySize;
		_timeoutSec = other._timeoutSec;
		_maxEvents = other._maxEvents;
	}
	return *this;
}

void Config::setup(const std::string &configFile) {
	(void)configFile;

	_listens.clear();
	{
		Listen l1;
		l1.interface = "0.0.0.0";
		l1.port = 8080;
		_listens.push_back(l1);

		Listen l2;
		l2.interface = "127.0.0.1";
		l2.port = 3000;
		_listens.push_back(l2);
	}

	_locations.clear();
	{
		Location loc1;
		loc1.path = "/";
		loc1.root = "/tmp/www";
		loc1.allowedMethods = {"GET", "POST", "DELETE"};
		loc1.autoindex = true;
		loc1.redirectUrl = "";
		loc1.indexFile = "index.html";
		loc1.errorFile = "/tmp/www/error.html";
		loc1.uploadStore = "/tmp/uploads";
		loc1.cgiConf.insert(std::make_pair(".php", "/usr/bin/php-cgi"));
		_locations.push_back(loc1);

		Location loc2;
		loc2.path = "/uploads";
		loc2.root = "/tmp/uploads";
		loc2.allowedMethods = {"GET", "POST"};
		loc2.autoindex = false;
		loc2.redirectUrl = "";
		loc2.indexFile = "upload.html";
		loc2.errorFile = "/tmp/www/error.html";
		loc2.uploadStore = "/tmp/uploads";
		loc2.cgiConf.insert(std::make_pair(".py", "/usr/bin/python3"));
		_locations.push_back(loc2);
	}

	_maxRequestBodySize = 1024 * 1024; // 1MB
	_timeoutSec = 5;
	_maxEvents = 1024;
}

const std::vector<Listen> &Config::getListens() const { return _listens; }
const std::vector<Location> &Config::getLocations() const { return _locations; }
unsigned int Config::getMaxRequestBodySize() const { return _maxRequestBodySize; }
unsigned int Config::getTimeoutSec() const { return _timeoutSec; }
unsigned int Config::getMaxEvents() const { return _maxEvents; }

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << "Config:\n";
	os << "  listens:\n";
	for (std::vector<Listen>::const_iterator it = config._listens.begin();
		 it != config._listens.end(); ++it) {
		os << "	- " << it->interface << ":" << it->port << "\n";
	}
	os << "  locations:\n";
	for (std::vector<Location>::const_iterator it = config._locations.begin();
		 it != config._locations.end(); ++it) {
		os << "	- path: " << it->path << "\n";
		os << "	  root: " << it->root << "\n";
		os << "	  allowedMethods: ";
		for (std::set<std::string>::const_iterator mit = it->allowedMethods.begin();
			 mit != it->allowedMethods.end(); ++mit) {
			os << *mit << " ";
		}
		os << "\n";
		os << "	  autoindex: " << it->autoindex << "\n";
		os << "	  redirectUrl: " << it->redirectUrl << "\n";
		os << "	  indexFile: " << it->indexFile << "\n";
		os << "	  errorFile: " << it->errorFile << "\n";
		os << "	  uploadStore: " << it->uploadStore << "\n";
		os << "	  cgiConf:\n";
		for (std::map<std::string, std::string>::const_iterator cit = it->cgiConf.begin();
			 cit != it->cgiConf.end(); ++cit) {
			os << "		" << cit->first << ": " << cit->second << "\n";
		}
	}
	os << "  maxRequestBodySize: " << config._maxRequestBodySize << "\n";
	os << "  timeoutSec: " << config._timeoutSec << "\n";
	os << "  maxEvents: " << config._maxEvents << "\n";
	return os;
}
