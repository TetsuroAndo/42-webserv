#include "CgiErrorExitEvent.hpp"

CgiErrorExitEvent::CgiErrorExitEvent(Client *client, CgiWorker &worker)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN),
	  ACgiEvent(worker) {}

CgiErrorExitEvent::~CgiErrorExitEvent() {}

void CgiErrorExitEvent::handle() { CgiHandle(); }

void CgiErrorExitEvent::process() { _worker.handleErrorExit(); }
