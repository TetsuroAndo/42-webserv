#include "CgiReadEvent.hpp"

#include "../../../Lib/Logger/Log.hpp"
#include "../../Server.hpp"

CgiReadEvent::CgiReadEvent(Client *client, CgiWorker &worker)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN),
	  ACgiEvent(worker) {}

CgiReadEvent::~CgiReadEvent() {}

void CgiReadEvent::handle() { CgiHandle(); }

void CgiReadEvent::process() {
	LOG(DEBUG) << "handle read" << attr("client fd", _client.getFd());
	_worker.handleRead();
}

void CgiReadEvent::close() {
	_client.getServer().getSocketsManager().unregisterSocket(_fd);
	::close(_fd);
}
