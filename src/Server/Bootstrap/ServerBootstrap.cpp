#include "ServerBootstrap.hpp"
#include <sstream>

namespace ServerBootstrap {

std::string listenToString(const Listen &listen) {
	std::ostringstream oss;
	oss << listen.interface << ":" << listen.port;
	return oss.str();
}

const Config &selectCgiConfig(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	const Config *selected = &configs[0];
	for (size_t i = 1; i < configs.size(); ++i) {
		if (configs[i].getTimeoutSec() > selected->getTimeoutSec()) {
			selected = &configs[i];
		}
	}
	return *selected;
}

size_t resolveMaxEvents(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	size_t maxEvents = configs[0].getMaxEvents();
	for (size_t i = 1; i < configs.size(); ++i) {
		maxEvents = std::max(maxEvents, configs[i].getMaxEvents());
	}
	return maxEvents;
}

size_t resolveMaxSessionTimeout(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	size_t maxTimeout = configs[0].getSessionTimeoutSec();
	for (size_t i = 1; i < configs.size(); ++i) {
		maxTimeout = std::max(maxTimeout, configs[i].getSessionTimeoutSec());
	}
	return maxTimeout;
}

void validateListenUniqueness(const std::vector< Config > &configs) {
	std::set< std::pair< std::string, int > > seen;
	std::set< int > wildcardPorts;
	std::set< int > anyPorts;

	for (size_t i = 0; i < configs.size(); ++i) {
		const std::vector< Listen > &listens = configs[i].getListens();
		if (listens.empty()) {
			throw std::runtime_error(
				"Config error: at least one listen is required per server");
		}
		for (size_t j = 0; j < listens.size(); ++j) {
			const Listen &listen = listens[j];
			const std::pair< std::string, int > key(listen.interface,
													listen.port);
			if (seen.count(key)) {
				throw std::runtime_error("Config error: duplicate listen " +
										 listenToString(listen));
			}
			const bool isWildcard = (listen.interface == "0.0.0.0");
			if (isWildcard) {
				if (anyPorts.count(listen.port)) {
					std::ostringstream oss;
					oss << listen.port;
					throw std::runtime_error(
						"Config error: wildcard listen conflicts with "
						"existing listen on port " +
						oss.str());
				}
				wildcardPorts.insert(listen.port);
				anyPorts.insert(listen.port);
			} else {
				if (wildcardPorts.count(listen.port)) {
					std::ostringstream oss;
					oss << listen.port;
					throw std::runtime_error(
						"Config error: listen " + listenToString(listen) +
						" conflicts with wildcard listen on port " + oss.str());
				}
				anyPorts.insert(listen.port);
			}
			seen.insert(key);
		}
	}
}

} // namespace ServerBootstrap
