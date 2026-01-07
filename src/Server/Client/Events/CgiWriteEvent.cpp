#include "CgiWriteEvent.hpp"
#include "../../../Lib/Logger/Log.hpp"
#include "../../Server.hpp"

CgiWriteEvent::CgiWriteEvent(CgiWorker &worker)
	: AEvent(EPOLLOUT), ACgiEvent(worker) {}

CgiWriteEvent::~CgiWriteEvent() {}

void CgiWriteEvent::handle() { CgiHandle(); }

void CgiWriteEvent::process() { _worker.handleWrite(); }
