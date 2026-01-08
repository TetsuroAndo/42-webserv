#include "ListenKey.hpp"
#include <sstream>

bool ListenKey::operator<(const ListenKey &other) const {
	if (interface < other.interface)
		return true;
	if (interface > other.interface)
		return false;
	return port < other.port;
}

bool ListenKey::operator==(const ListenKey &other) const {
	return interface == other.interface && port == other.port;
}

std::string ListenKey::listenKeyToString(const ListenKey &key) {
	std::ostringstream oss;
	oss << key.interface << ":" << key.port;
	return oss.str();
}
