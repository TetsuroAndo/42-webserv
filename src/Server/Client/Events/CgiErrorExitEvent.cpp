#include "CgiErrorExitEvent.hpp"

CgiErrorExitEvent::CgiErrorExitEvent(CgiWorker &worker)
	: AEvent(EPOLLIN), ACgiEvent(worker) {}

CgiErrorExitEvent::~CgiErrorExitEvent() {}

void CgiErrorExitEvent::handle() { CgiHandle(); }

void CgiErrorExitEvent::process() { _worker.handleErrorExit(); }
