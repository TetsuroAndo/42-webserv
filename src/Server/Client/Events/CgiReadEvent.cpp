#include "CgiReadEvent.hpp"

#include "../../Server.hpp"

CgiReadEvent::CgiReadEvent(CgiWorker *worker)
	: AEvent(EPOLLIN), ACgiEvent(worker) {}

CgiReadEvent::~CgiReadEvent() {}

void CgiReadEvent::handle() { CgiHandle(); }

void CgiReadEvent::process() { _worker->handleRead(); }
