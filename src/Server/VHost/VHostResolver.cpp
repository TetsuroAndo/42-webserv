#include "VHostResolver.hpp"
#include "../../Lib/StringOps/StringOps.hpp"

namespace {

std::string extractHostForMatch(const std::string &value) {
	std::string trimmed = StringOps::trim(value);
	if (trimmed.empty()) {
		return std::string();
	}
	std::string host;
	size_t colon = trimmed.find(':');
	if (colon != std::string::npos) {
		host = trimmed.substr(0, colon);
	} else {
		host = trimmed;
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
	if (vhosts.empty()) {
		return NULL;
	}
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
			const std::string listenHost =
				extractHostForMatch(listen.host);
			if (defaultVhost == NULL || listenHost.empty()) {
				defaultVhost = &vhost;
			}
			if (!headerHost.empty()) {
				if (!listenHost.empty() && listenHost == headerHost) {
					return &vhost;
				}
			}
		}
	}
	return defaultVhost;
}
