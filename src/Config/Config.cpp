#include "Config.hpp"

#include <iostream>
#include <ostream>
#include <string>

void Config::setListens(const std::vector<Listen> &lists) {
	this->_listens = lists;
}

void Config::setDefaultErrorPage(const std::string &page) {
	this->_defaultErrorPage = page;
}

void Config::setMaxRequestBodySize(unsigned int size) {
	this->_maxRequestBodySize = size;
}

void Config::setIsAllowGet(bool allow) { this->_isAllowGet = allow; }

void Config::setIsAllowPost(bool allow) { this->_isAllowPost = allow; }

void Config::setIsAllowHead(bool allow) { this->_isAllowHead = allow; }
void Config::setIsAllowDelete(bool allow) { this->_isAllowDelete = allow; }
void Config::setRedirect(const std::string &url) {
	this->_redirect = url;
}
void Config::setLocations(const std::vector<Location> &url) {
	this->_locations = url;
}
void Config::setIsShowDirectoryListPage(bool show) {
	this->_isShowDirectoryListPage = show;
}
void Config::setWhenRequestedDirectory(const std::string &dir) {
	this->_whenRequestedDirectory = dir;
}
void Config::setSaveFileDirectory(const std::string &dir) {
	this->_saveFileDirectory = dir;
}
Config::Config() { setup(); }

Config::Config(std::string configFile) {
	setup(configFile);
}

Config::~Config() {}
Config::Config(const Config &other)
	: _listens(other._listens), _defaultErrorPage(other._defaultErrorPage),
	  _maxRequestBodySize(other._maxRequestBodySize),
	  _isAllowGet(other._isAllowGet), _isAllowPost(other._isAllowPost),
	  _isAllowHead(other._isAllowHead), _isAllowDelete(other._isAllowDelete),
	  _redirect(other._redirect), _locations(other._locations),
	  _isShowDirectoryListPage(other._isShowDirectoryListPage),
	  _whenRequestedDirectory(other._whenRequestedDirectory),
	  _saveFileDirectory(other._saveFileDirectory), _timeoutSec(other._timeoutSec),
	  _maxEvents(other._maxEvents) {}
Config &Config::operator=(const Config &other) {
	if (this != &other) {
		_listens = other._listens;
		_defaultErrorPage = other._defaultErrorPage;
		_maxRequestBodySize = other._maxRequestBodySize;
		_isAllowGet = other._isAllowGet;
		_isAllowPost = other._isAllowPost;
		_isAllowHead = other._isAllowHead;
		_isAllowDelete = other._isAllowDelete;
		_redirect = other._redirect;
		_locations = other._locations;
		_isShowDirectoryListPage = other._isShowDirectoryListPage;
		_whenRequestedDirectory = other._whenRequestedDirectory;
		_saveFileDirectory = other._saveFileDirectory;
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

	_defaultErrorPage = "/tmp/www/error.html";
	_maxRequestBodySize = 1024 * 1024; // 1MB
	_isAllowGet = true;
	_isAllowPost = true;
	_isAllowHead = true;
	_isAllowDelete = true;
	_redirect = "";

	_locations.clear();
	{
		Location loc1;
		loc1.path = "/";
		loc1.root = "/tmp/www";
		_locations.push_back(loc1);

		Location loc2;
		loc2.path = "/uploads";
		loc2.root = "/tmp/uploads";
		_locations.push_back(loc2);
	}

	_isShowDirectoryListPage = true;
	_whenRequestedDirectory = "index.html";
	_saveFileDirectory = "/tmp/uploads";
	_timeoutSec = 5;
	_maxEvents = 1024;
}
const std::vector<Listen> &Config::getListens() const { return _listens; }
const std::string &Config::getDefaultErrorPage() const {
	return _defaultErrorPage;
}

unsigned int Config::getMaxRequestBodySize() const {
	return _maxRequestBodySize;
}
bool Config::getIsAllowGet() const { return _isAllowGet; }
bool Config::getIsAllowPost() const { return _isAllowPost; }
bool Config::getIsAllowHead() const { return _isAllowHead; }
bool Config::getIsAllowDelete() const { return _isAllowDelete; }
const std::string &Config::getRedirect() const { return _redirect; }
const std::vector<Location> &Config::getLocations() const { return _locations; }
bool Config::getIsShowDirectoryListPage() const {
	return _isShowDirectoryListPage;
}
const std::string &Config::getWhenRequestedDirectory() const {
	return _whenRequestedDirectory;
}
const std::string &Config::getSaveFileDirectory() const {
	return _saveFileDirectory;
}
unsigned int Config::getTimeoutSec() const { return _timeoutSec; }
unsigned int Config::getMaxEvents() const { return _maxEvents; }

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << "Config:\n";
	os << "  listens:\n";
	for (std::vector<Listen>::const_iterator it = config._listens.begin();
		 it != config._listens.end(); ++it) {
		os << "    - " << it->interface << ":" << it->port << "\n";
	}
	os << "  defaultErrorPage: " << config._defaultErrorPage << "\n";
	os << "  maxRequestBodySize: " << config._maxRequestBodySize << "\n";
	os << "  isAllowGet: " << config._isAllowGet << "\n";
	os << "  isAllowPost: " << config._isAllowPost << "\n";
	os << "  isAllowHead: " << config._isAllowHead << "\n";
	os << "  isAllowDelete: " << config._isAllowDelete << "\n";
	os << "  redirect: " << config._redirect << "\n";
	os << "  locations:\n";
	for (std::vector<Location>::const_iterator it = config._locations.begin();
		 it != config._locations.end(); ++it) {
		os << "    - path: " << it->path << ", root: " << it->root << "\n";
	}
	os << "  isShowDirectoryListPage: " << config._isShowDirectoryListPage
	   << "\n";
	os << "  whenRequestedDirectory: " << config._whenRequestedDirectory << "\n";
	os << "  saveFileDirectory: " << config._saveFileDirectory << "\n";
	os << "  timeoutSec: " << config._timeoutSec << "\n";
	os << "  maxEvents: " << config._maxEvents << "\n";
	return os;
}
