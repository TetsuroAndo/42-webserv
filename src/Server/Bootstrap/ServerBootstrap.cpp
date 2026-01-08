#include "ServerBootstrap.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <sstream>

namespace ServerBootstrap {

std::string listenToString(const Listen &listen) {
	std::ostringstream oss;
	oss << listen.interface << ":" << listen.port;
	return oss.str();
}

size_t resolveCgiMaxWorkers(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	size_t maxWorkers = 0;
	for (size_t i = 0; i < configs.size(); ++i) {
		const Config &config = configs[i];
		const size_t resolved = std::max(
			config.getPerformance().cgiMinWorkers,
			std::min(config.getMaxEvents(),
					 config.getPerformance().cgiMaxWorkers));
		maxWorkers = std::max(maxWorkers, resolved);
	}
	return maxWorkers;
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

std::map< std::string, size_t >
resolveListenHeaderMax(const std::vector< Config > &configs) {
	if (configs.empty()) {
		throw std::runtime_error("Server error: no servers configured");
	}
	std::map< std::string, size_t > headerMaxByListen;
	for (size_t i = 0; i < configs.size(); ++i) {
		const Config &config = configs[i];
		const std::vector< Listen > &listens = config.getListens();
		for (size_t j = 0; j < listens.size(); ++j) {
			const Listen &listen = listens[j];
			const std::string key = listenToString(listen);
			const size_t maxHeader = config.getMaxRequestHeaderSize();
			std::map< std::string, size_t >::iterator it =
				headerMaxByListen.find(key);
			if (it == headerMaxByListen.end() || it->second < maxHeader) {
				headerMaxByListen[key] = maxHeader;
			}
		}
	}
	return headerMaxByListen;
}

void validateListenCompatibility(const std::vector< Config > &configs) {
	std::set< std::pair< std::string, int > > seen;
	std::set< int > wildcardPorts;
	std::set< int > specificPorts;

	for (size_t i = 0; i < configs.size(); ++i) {
		const std::vector< Listen > &listens = configs[i].getListens();
		if (listens.empty()) {
			throw std::runtime_error(
				"Config error: at least one listen is required per server");
		}
		for (size_t j = 0; j < listens.size(); ++j) {
			const Listen &listen = listens[j];
			// Host name ロジックを追加する場合は、この処理を変更する
			const std::pair< std::string, int > key(listen.interface,
													listen.port);
			if (seen.count(key)) {
				throw std::runtime_error("Config error: duplicate listen " +
										 listenToString(listen));
			}
			const bool isWildcard = (listen.interface == "0.0.0.0");
			if (isWildcard) {
				if (specificPorts.count(listen.port)) {
					std::ostringstream oss;
					oss << listen.port;
					throw std::runtime_error(
						"Config error: wildcard listen conflicts with "
						"existing listen on port " +
						oss.str());
				}
				wildcardPorts.insert(listen.port);
			} else {
				if (wildcardPorts.count(listen.port)) {
					std::ostringstream oss;
					oss << listen.port;
					throw std::runtime_error(
						"Config error: listen " + listenToString(listen) +
						" conflicts with wildcard listen on port " + oss.str());
				}
				specificPorts.insert(listen.port);
			}
			seen.insert(key);
		}
	}
}

} // namespace ServerBootstrap
