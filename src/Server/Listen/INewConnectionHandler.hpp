#pragma once

class INewConnectionHandler {
public:
	virtual ~INewConnectionHandler() {}
	virtual void handleNewConnection(int listenFd) = 0;
};
