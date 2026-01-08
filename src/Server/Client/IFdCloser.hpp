#pragma once

class IFdCloser {
public:
	virtual ~IFdCloser() {}
	virtual void closeFd(int fd) = 0;
};
