#pragma once

#include <string>

struct ListenKey {
	std::string interface;
	int port;

	ListenKey() : interface(), port(0) {}
	ListenKey(const std::string &iface, int p) : interface(iface), port(p) {}

	bool operator<(const ListenKey &other) const {
		if (interface < other.interface) {
			return true;
		}
		if (interface > other.interface) {
			return false;
		}
		return port < other.port;
	}

	bool operator==(const ListenKey &other) const {
		return interface == other.interface && port == other.port;
	}
};
