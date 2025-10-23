#pragma once

enum FdChangeType {
	FdChangeType_ADD,
	FdChangeType_REMOVE,
	FdChangeType_NOTIFY
};

struct FdEventChange {
	int fd;
	int eventType;
	FdChangeType changeType;
};
