#include "VHostResolver.hpp"
#include "../../Lib/StringOps/StringOps.hpp"

namespace {

std::string extractHostForMatch(const std::string &value) {
	std::string trimmed = StringOps::trim(value);
	if (trimmed.empty()) {
		return std::string();
	}
	std::string host;
	if (trimmed[0] == '[') {
		size_t end = trimmed.find(']');
		if (end != std::string::npos && end > 1) {
			host = trimmed.substr(1, end - 1);
		} else {
			host = trimmed;
		}
	} else {
		size_t colon = trimmed.find(':');
		if (colon != std::string::npos &&
			trimmed.find(':', colon + 1) == std::string::npos) {
			host = trimmed.substr(0, colon);
		} else {
			host = trimmed;
		}
	}
	StringOps::trim(host);
	StringOps::toLower(host);
	return host;
}

} // namespace

VirtualHost *VHostResolver::find(std::vector< VirtualHost > &vhosts,
								 const ListenKey &listenKey,
								 const std::string &hostHeader) {
	VirtualHost *defaultVhost = NULL;
	const std::string headerHost = extractHostForMatch(hostHeader);

	for (size_t i = 0; i < vhosts.size(); ++i) {
		VirtualHost &vhost = vhosts[i];
		const std::vector< Listen > &listens = vhost.config.getListens();
		for (size_t j = 0; j < listens.size(); ++j) {
			const Listen &listen = listens[j];
			if (listen.interface != listenKey.interface ||
				listen.port != listenKey.port) {
				continue;
			}
			if (defaultVhost == NULL) {
				defaultVhost = &vhost;
			}
			if (!headerHost.empty()) {
				const std::string listenHost =
					extractHostForMatch(listen.host);
				if (!listenHost.empty() && listenHost == headerHost) {
					return &vhost;
				}
			}
		}
	}
	return defaultVhost;
}
