#pragma once

#include "../VHost/VirtualHost.hpp"
#include "INewConnectionHandler.hpp"
#include "Listener.hpp"
#include <map>
#include <vector>

class EventManager;
class SocketsManager;

class ListenerSet {
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
			  addr(),
			  key(),
			  defaultVhostIndex(0),
			  found(false) {}
	};
	// clang-format on

	ListenerSet();
	~ListenerSet();

	void build(const std::vector< VirtualHost > &vhosts,
			   SocketsManager &socketsManager, EventManager &eventManager,
			   INewConnectionHandler &handler);

	AcceptedConn acceptOnce(int listenFd) const;
	void forgetAll(EventManager &eventManager);

private:
	std::map< ListenKey, Listener > _listeners;
	std::map< int, Listener * > _listenersByFd;

	void openAndRegisterListeners(SocketsManager &socketsManager,
								  EventManager &eventManager,
								  INewConnectionHandler &handler);
};
