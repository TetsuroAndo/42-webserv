#include "CgiErrorEvent.hpp"

#include "../../../Lib/Logger/ErrorLog/LogBuilder.hpp"
#include "../../Server.hpp"

CgiErrorEvent::CgiErrorEvent(CgiWorker &worker)
	: AEvent(EPOLLIN), ACgiEvent(worker) {}

CgiErrorEvent::~CgiErrorEvent() {}

void CgiErrorEvent::handle() { CgiHandle(); }

void CgiErrorEvent::process() { _worker.handleReadErr(); }
