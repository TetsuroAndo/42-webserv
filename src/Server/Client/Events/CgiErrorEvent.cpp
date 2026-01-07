#include "CgiErrorEvent.hpp"

#include "../../../Lib/Logger/ErrorLog/LogBuilder.hpp"
#include "../../Server.hpp"

CgiErrorEvent::CgiErrorEvent(Client *client, CgiWorker &worker)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN),
	  ACgiEvent(worker) {}

CgiErrorEvent::~CgiErrorEvent() {}

void CgiErrorEvent::handle() { CgiHandle(); }

void CgiErrorEvent::process() {
	LOG(DEBUG) << "handle error" << attr("client fd", _client->getFd());
	_worker.handleReadErr();
}
