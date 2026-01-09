#pragma once

#include "../VHost/VirtualHost.hpp"
#include "INewConnectionHandler.hpp"
#include "Listener.hpp"
#include <cstring>
#include <map>
#include <vector>

class EventManager;
class SocketsManager;

class ListenerRegistry {
public:
	// clang-format off
	struct AcceptedConn {
		int fd;
		sockaddr_in addr;
		ListenKey key;
		size_t defaultVhostIndex;
		bool found;

		AcceptedConn()
			: fd(-1),
			  key(),
			  defaultVhostIndex(0),
			  found(false) {
			std::memset(&addr, 0, sizeof(addr));
		}
	};
	// clang-format on

	ListenerRegistry();
	~ListenerRegistry();

	void build(const std::vector< VirtualHost > &vhosts,
			   SocketsManager &socketsManager, EventManager &eventManager,
			   INewConnectionHandler &handler);

	AcceptedConn acceptOnce(int listenFd) const;
	void forgetAll(EventManager &eventManager, SocketsManager &socketsManager);

private:
	std::map< ListenKey, Listener > _listeners;
	std::map< int, Listener * > _listenersByFd;

	void openAndRegisterListeners(SocketsManager &socketsManager,
								  EventManager &eventManager,
								  INewConnectionHandler &handler);
};
