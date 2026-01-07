#include "CgiWriteEvent.hpp"
#include "../../../Lib/Logger/Log.hpp"
#include "../../Server.hpp"

CgiWriteEvent::CgiWriteEvent(Client *client, CgiWorker &worker)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLOUT),
	  ACgiEvent(worker) {}

CgiWriteEvent::~CgiWriteEvent() {}

void CgiWriteEvent::handle() { CgiHandle(); }

void CgiWriteEvent::process() {
	LOG(DEBUG) << "handle write" << attr("client fd", _client->getFd());
	_worker.handleWrite();
}
