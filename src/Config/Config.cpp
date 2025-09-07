#include "Config.hpp"

#include <ostream>

Config::Config() { port = 8080; }

Config::~Config() {}

Config::Config(const Config &other) { port = other.port; }

Config &Config::operator=(const Config &other) {
	if (this != &other) {
		port = other.port;
	}
	return *this;
}

int Config::getPort() const { return port; }

void Config::setPort(const int newPort) { this->port = newPort; }

std::ostream &operator<<(std::ostream &os, const Config &config) {
	os << config.getPort();
	return os;
}