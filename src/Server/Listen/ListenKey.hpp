#pragma once

#include <string>

struct ListenKey {
	std::string interface;
	int port;

	ListenKey() : interface(), port(0) {}
	ListenKey(const std::string &iface, int p) : interface(iface), port(p) {}

	bool operator<(const ListenKey &other) const;
	bool operator==(const ListenKey &other) const;

	static std::string listenKeyToString(const ListenKey &key);
};
