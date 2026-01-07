#include "CgiReadEvent.hpp"

#include "../../../Lib/Logger/Log.hpp"
#include "../../Server.hpp"

CgiReadEvent::CgiReadEvent(CgiWorker &worker)
	: AEvent(EPOLLIN), ACgiEvent(worker) {}

CgiReadEvent::~CgiReadEvent() {}

void CgiReadEvent::handle() { CgiHandle(); }

void CgiReadEvent::process() { _worker.handleRead(); }
