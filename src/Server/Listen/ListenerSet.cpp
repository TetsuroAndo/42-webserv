#include "ListenerSet.hpp"

#include "../../Lib/Logger/Log.hpp"
#include "../../Socket/SocketsManager.hpp"
#include "../Client/EventManager.hpp"
#include "../Client/Events/NewConnectionEvent.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
#include <vector>

ListenerSet::ListenerSet() : _listeners(), _listenersByFd() {}

ListenerSet::~ListenerSet() {}

void ListenerSet::build(const std::vector< VirtualHost > &vhosts,
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
				it =
					_listeners.insert(std::make_pair(key, Listener(key))).first;
			}
			it->second.addVhostIndex(i);
		}
	}

	openAndRegisterListeners(socketsManager, eventManager, handler);
}

ListenerSet::AcceptedConn ListenerSet::acceptOnce(const int listenFd) const {
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
	if (flags < 0 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) < 0) {
		close(clientFd);
		return result;
	}

	result.fd = clientFd;
	result.addr = clientAddr;
	return result;
}

void ListenerSet::forgetAll(EventManager &eventManager,
							SocketsManager &socketsManager) {
	for (std::map< int, Listener * >::iterator it = _listenersByFd.begin();
		 it != _listenersByFd.end(); ++it) {
		const int fd = it->first;
		// デストラクタ経路のため、ここは例外を投げない（terminate回避）。
		try {
			eventManager.forgetFd(fd);
		} catch (...) {
		}
		try {
			socketsManager.unregisterSocket(fd);
		} catch (...) {
		}
		if (fd >= 0) {
			close(fd);
		}
		if (it->second) {
			sockaddr_in empty;
			std::memset(&empty, 0, sizeof(empty));
			it->second->setSocket(-1, empty);
		}
	}
	_listenersByFd.clear();
	_listeners.clear();
}

void ListenerSet::openAndRegisterListeners(SocketsManager &socketsManager,
										   EventManager &eventManager,
										   INewConnectionHandler &handler) {
	std::vector< int > openedFds;
	int currentFd = -1;

	struct ScopedFd {
		int fd;
		explicit ScopedFd(int fdIn) : fd(fdIn) {}
		~ScopedFd() {
			if (fd >= 0) {
				close(fd);
			}
		}
		void release() { fd = -1; }
	};

	try {
		for (std::map< ListenKey, Listener >::iterator it = _listeners.begin();
			 it != _listeners.end(); ++it) {
			currentFd = -1;

			const ListenKey &key = it->first;
			const int listenFd = socket(AF_INET, SOCK_STREAM, 0);
			if (listenFd < 0) {
				LOG(FATAL) << "socket() failed: " << strerror(errno);
				throw std::runtime_error("socket() failed");
			}
			ScopedFd listenGuard(listenFd);
			currentFd = listenFd;

			const int flags = fcntl(listenFd, F_GETFL, 0);
			if (flags < 0 || fcntl(listenFd, F_SETFL, flags | O_NONBLOCK) < 0) {
				LOG(FATAL) << "fcntl() failed for listen socket: "
						   << strerror(errno);
				throw std::runtime_error("fcntl() failed");
			}

			int opt = 1;
			if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt,
						   sizeof(opt)) < 0) {
				LOG(FATAL) << "setsockopt() failed: " << strerror(errno);
				throw std::runtime_error("setsockopt() failed");
			}

			sockaddr_in addr;
			std::memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(key.port);

			addrinfo hints;
			std::memset(&hints, 0, sizeof(hints));
			addrinfo *res = NULL;
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

			int ret = getaddrinfo(key.interface.c_str(), NULL, &hints, &res);
			if (ret != 0) {
				LOG(FATAL) << "getaddrinfo() failed for "
						   << key.interface << ": " << gai_strerror(ret);
				throw std::runtime_error("getaddrinfo() failed");
			}

			std::memcpy(
				&addr.sin_addr,
				&reinterpret_cast< sockaddr_in * >(res->ai_addr)->sin_addr,
				sizeof(addr.sin_addr));
			freeaddrinfo(res);

			if (bind(listenFd, reinterpret_cast< sockaddr * >(&addr),
					 sizeof(addr)) < 0) {
				LOG(FATAL) << "bind() failed for " << key.interface << ":"
						   << key.port << ": " << strerror(errno);
				throw std::runtime_error("bind() failed");
			}
			if (listen(listenFd, SOMAXCONN) < 0) {
				LOG(FATAL) << "listen() failed for " << key.interface << ":"
						   << key.port << ": " << strerror(errno);
				throw std::runtime_error("listen() failed");
			}

			it->second.setSocket(listenFd, addr);

			socketsManager.registerSocket(listenFd, EPOLLIN);
			eventManager.initFd(listenFd);

			std::auto_ptr< NewConnectionEvent > event(
				new NewConnectionEvent(handler));
			_listenersByFd[listenFd] = &it->second;
			eventManager.addEvent(listenFd, event.get());
			event.release();
			openedFds.push_back(listenFd);
			listenGuard.release();
			currentFd = -1;

			LOG(INFO) << "Listening on " << key.interface << ":" << key.port
					  << attr("fd", listenFd);
		}
	} catch (...) {
		if (currentFd >= 0) {
			// ロールバック中の二次例外で元の例外を潰さないよう、ここも例外を投げない。
			try {
				eventManager.forgetFd(currentFd);
			} catch (...) {
			}
			try {
				socketsManager.unregisterSocket(currentFd);
			} catch (...) {
			}
		}

		for (size_t i = 0; i < openedFds.size(); ++i) {
			const int fd = openedFds[i];
			// ロールバック中の二次例外で元の例外を潰さないよう、ここも例外を投げない。
			try {
				eventManager.forgetFd(fd);
			} catch (...) {
			}
			try {
				socketsManager.unregisterSocket(fd);
			} catch (...) {
			}
			close(fd);
		}

		_listenersByFd.clear();
		for (std::map< ListenKey, Listener >::iterator it = _listeners.begin();
			 it != _listeners.end(); ++it) {
			sockaddr_in empty;
			std::memset(&empty, 0, sizeof(empty));
			it->second.setSocket(-1, empty);
		}
		throw;
	}
}
