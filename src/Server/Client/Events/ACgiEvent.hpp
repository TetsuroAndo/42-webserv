#pragma once
#include "../../../Cgi/CgiWorker.hpp"

#include <unistd.h>

class ACgiEvent {
public:
	ACgiEvent(CgiWorker *worker) : _worker(worker) {
		if (worker == NULL) {
			throw std::runtime_error("ACgiEvent: NULL worker");
		}
	}
	virtual ~ACgiEvent() {}

	void CgiHandle() {
		if (_worker == NULL)
			return;
		_worker->updateLastActivityTime();
		process();
	}

	/// @brief CgiEventの共通部分ではない部分
	virtual void process() = 0;

protected:
	CgiWorker *_worker;
};
