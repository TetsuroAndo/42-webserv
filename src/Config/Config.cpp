#include "Config.hpp"

#include <ostream>

#include "Config.hpp"

#include <iostream>

Config::Config() { setup(); }

Config::~Config() {}

Config::Config(const Config &other)
	: listens(other.listens), defaultErrorPage(other.defaultErrorPage),
	  maxRequestBodySize(other.maxRequestBodySize),
	  isAllowGet(other.isAllowGet), isAllowPost(other.isAllowPost),
	  isAllowHead(other.isAllowHead), isAllowDelete(other.isAllowDelete),
	  redirect(other.redirect), locations(other.locations),
	  isShowDirectoryListPage(other.isShowDirectoryListPage),
	  whenRequestedDirectory(other.whenRequestedDirectory),
	  saveFileDirectory(other.saveFileDirectory), timeoutSec(other.timeoutSec),
	  maxEvents(other.maxEvents) {}

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		listens = other.listens;
		defaultErrorPage = other.defaultErrorPage;
		maxRequestBodySize = other.maxRequestBodySize;
		isAllowGet = other.isAllowGet;
		isAllowPost = other.isAllowPost;
		isAllowHead = other.isAllowHead;
		isAllowDelete = other.isAllowDelete;
		redirect = other.redirect;
		locations = other.locations;
		isShowDirectoryListPage = other.isShowDirectoryListPage;
		whenRequestedDirectory = other.whenRequestedDirectory;
		saveFileDirectory = other.saveFileDirectory;
		timeoutSec = other.timeoutSec;
		maxEvents = other.maxEvents;
	}
	return *this;
}

void Config::setup(std::string configFile) {
	(void)configFile;

	listens.clear();
	{
		listen l1;
		l1.interface = "0.0.0.0";
		l1.port = 8080;
		listens.push_back(l1);

		listen l2;
		l2.interface = "127.0.0.1";
		l2.port = 3000;
		listens.push_back(l2);
	}

	defaultErrorPage = "/tmp/www/error.html";
	maxRequestBodySize = 1024 * 1024; // 1MB
	isAllowGet = true;
	isAllowPost = true;
	isAllowHead = true;
	isAllowDelete = true;
	redirect = "";

	locations.clear();
	{
		location loc1;
		loc1.path = "/";
		loc1.root = "/tmp/www";
		locations.push_back(loc1);

		location loc2;
		loc2.path = "/uploads";
		loc2.root = "/tmp/uploads";
		locations.push_back(loc2);
	}

	isShowDirectoryListPage = true;
	whenRequestedDirectory = "index.html";
	saveFileDirectory = "/tmp/uploads";
	timeoutSec = 5;
	maxEvents = 1024;
}

const std::vector<listen> &Config::getListens() const { return listens; }
const std::string &Config::getDefaultErrorPage() const {
	return defaultErrorPage;
}
unsigned int Config::getMaxRequestBodySize() const {
	return maxRequestBodySize;
}
bool Config::getIsAllowGet() const { return isAllowGet; }
bool Config::getIsAllowPost() const { return isAllowPost; }
bool Config::getIsAllowHead() const { return isAllowHead; }
bool Config::getIsAllowDelete() const { return isAllowDelete; }
const std::string &Config::getRedirect() const { return redirect; }
const std::vector<location> &Config::getLocations() const { return locations; }
bool Config::getIsShowDirectoryListPage() const {
	return isShowDirectoryListPage;
}
const std::string &Config::getWhenRequestedDirectory() const {
	return whenRequestedDirectory;
}
const std::string &Config::getSaveFileDirectory() const {
	return saveFileDirectory;
}
unsigned int Config::getTimeoutSec() const { return timeoutSec; }
unsigned int Config::getMaxEvents() const { return maxEvents; }

void Config::setListens(const std::vector<listen> &listens) {
	this->listens = listens;
}
void Config::setDefaultErrorPage(const std::string &page) {
	this->defaultErrorPage = page;
}
void Config::setMaxRequestBodySize(unsigned int size) {
	this->maxRequestBodySize = size;
}
void Config::setIsAllowGet(bool allow) { this->isAllowGet = allow; }
void Config::setIsAllowPost(bool allow) { this->isAllowPost = allow; }
void Config::setIsAllowHead(bool allow) { this->isAllowHead = allow; }
void Config::setIsAllowDelete(bool allow) { this->isAllowDelete = allow; }
void Config::setRedirect(const std::string &redirect) {
	this->redirect = redirect;
}
void Config::setLocations(const std::vector<location> &locations) {
	this->locations = locations;
}
void Config::setIsShowDirectoryListPage(bool show) {
	this->isShowDirectoryListPage = show;
}
void Config::setWhenRequestedDirectory(const std::string &dir) {
	this->whenRequestedDirectory = dir;
}
void Config::setSaveFileDirectory(const std::string &dir) {
	this->saveFileDirectory = dir;
}

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << "Config:\n";
	os << "  listens:\n";
	for (std::vector<listen>::const_iterator it = config.listens.begin();
		 it != config.listens.end(); ++it) {
		os << "    - " << it->interface << ":" << it->port << "\n";
	}
	os << "  defaultErrorPage: " << config.defaultErrorPage << "\n";
	os << "  maxRequestBodySize: " << config.maxRequestBodySize << "\n";
	os << "  isAllowGet: " << config.isAllowGet << "\n";
	os << "  isAllowPost: " << config.isAllowPost << "\n";
	os << "  isAllowHead: " << config.isAllowHead << "\n";
	os << "  isAllowDelete: " << config.isAllowDelete << "\n";
	os << "  redirect: " << config.redirect << "\n";
	os << "  locations:\n";
	for (std::vector<location>::const_iterator it = config.locations.begin();
		 it != config.locations.end(); ++it) {
		os << "    - path: " << it->path << ", root: " << it->root << "\n";
	}
	os << "  isShowDirectoryListPage: " << config.isShowDirectoryListPage
	   << "\n";
	os << "  whenRequestedDirectory: " << config.whenRequestedDirectory << "\n";
	os << "  saveFileDirectory: " << config.saveFileDirectory << "\n";
	os << "  timeoutSec: " << config.timeoutSec << "\n";
	os << "  maxEvents: " << config.maxEvents << "\n";
	return os;
}
