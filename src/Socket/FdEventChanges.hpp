#pragma once

#include <vector>

struct FdEvent {
	int fd;
	int event_type; // EPOLLIN, EPOLLOUT など
};

struct FdEventChanges {
	std::vector< FdEvent > fdsToAdd;
	std::vector< int > fdsToRemove;
	std::vector< int >
		clientFdsToNotify; // CGI完了時にレスポンスを送るべきclientFd
};
