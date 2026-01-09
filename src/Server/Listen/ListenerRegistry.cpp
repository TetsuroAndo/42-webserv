#include "ListenerRegistry.hpp"

#include "../../Lib/Logger/Log.hpp"
#include "../Client/EventManager.hpp"
#include "../Client/Events/NewConnectionEvent.hpp"
#include "../../Socket/SocketsManager.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <utility>
#include <unistd.h>

ListenerRegistry::ListenerRegistry() : _listeners(), _listenersByFd() {}

ListenerRegistry::~ListenerRegistry() {}

void ListenerRegistry::build(const std::vector< VirtualHost > &vhosts,
						SocketsManager &socketsManager,
						EventManager &eventManager,
						INewConnectionHandler &handler) {
	_listeners.clear();
	_listenersByFd.clear();

	for (size_t i = 0; i < vhosts.size(); ++i) {
		const Config &config = vhosts[i].config;
		const std::vector< Listen > &listens = config.getListens();

		for (size_t j = 0; j < listens.size(); ++j) {
			const Listen &listenConf = listens[j];

			ListenKey key(listenConf.interface, listenConf.port);
			std::map< ListenKey, Listener >::iterator it = _listeners.find(key);

			if (it == _listeners.end()) {
				it = _listeners.insert(std::make_pair(key, Listener(key))).first;
			}
			it->second.addVhostIndex(i);
		}
	}

	openAndRegisterListeners(socketsManager, eventManager, handler);
}

ListenerRegistry::AcceptedConn ListenerRegistry::acceptOnce(const int listenFd) const {
	AcceptedConn result;
	std::map< int, Listener * >::const_iterator it =
		_listenersByFd.find(listenFd);
	if (it == _listenersByFd.end() || it->second == NULL) {
		return result;
	}

	const Listener *listener = it->second;
	result.found = true;
	result.key = listener->getKey();
	result.defaultVhostIndex = listener->getDefaultVhostIndex();

	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	const int clientFd =
		accept(listenFd, reinterpret_cast< struct sockaddr * >(&clientAddr),
			   &clientLen);

	if (clientFd < 0) {
		return result;
	}

	const int flags = fcntl(clientFd, F_GETFL, 0);
	fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

	result.fd = clientFd;
	result.addr = clientAddr;
	return result;
}

void ListenerRegistry::forgetAll(EventManager &eventManager) {
	for (std::map< int, Listener * >::iterator it = _listenersByFd.begin();
		 it != _listenersByFd.end(); ++it) {
		eventManager.forgetFd(it->first);
	}
	_listenersByFd.clear();
	_listeners.clear();
}

void ListenerRegistry::openAndRegisterListeners(SocketsManager &socketsManager,
										  EventManager &eventManager,
										  INewConnectionHandler &handler) {
	for (std::map< ListenKey, Listener >::iterator it = _listeners.begin();
		 it != _listeners.end(); ++it) {
		const ListenKey &key = it->first;
		int listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd < 0) {
			LOG(FATAL) << "socket() failed: " << strerror(errno);
			throw std::runtime_error("socket() failed");
		}

		const int flags = fcntl(listenFd, F_GETFL, 0);
		fcntl(listenFd, F_SETFL, flags | O_NONBLOCK);

		int opt = 1;
		setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(key.port);

		addrinfo hints = {}, *res;
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

		int ret = getaddrinfo(key.interface.c_str(), NULL, &hints, &res);
		if (ret != 0) {
			close(listenFd);
			LOG(FATAL) << "getaddrinfo() failed for " << key.interface << ": "
					   << gai_strerror(ret);
			throw std::runtime_error("getaddrinfo() failed");
		}

		std::memcpy(&addr.sin_addr,
					&reinterpret_cast< sockaddr_in * >(res->ai_addr)->sin_addr,
					sizeof(addr.sin_addr));
		freeaddrinfo(res);

		if (bind(listenFd, reinterpret_cast< sockaddr * >(&addr),
				 sizeof(addr)) < 0) {
			close(listenFd);
			LOG(FATAL) << "bind() failed for " << key.interface << ":"
					   << key.port << ": " << strerror(errno);
			throw std::runtime_error("bind() failed");
		}
		if (listen(listenFd, SOMAXCONN) < 0) {
			close(listenFd);
			LOG(FATAL) << "listen() failed for " << key.interface << ":"
					   << key.port << ": " << strerror(errno);
			throw std::runtime_error("listen() failed");
		}

		it->second.setSocket(listenFd, addr);
		_listenersByFd[listenFd] = &it->second;
		socketsManager.registerSocket(listenFd, EPOLLIN);
		eventManager.initFd(listenFd);
		eventManager.addEvent(listenFd, new NewConnectionEvent(handler));
		LOG(INFO) << "Listening on " << key.interface << ":" << key.port
				  << attr("fd", listenFd);
	}
}
