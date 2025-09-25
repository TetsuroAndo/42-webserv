#include "TimeCache.hpp"
#include <ctime>

std::string TimeCache::_cachedTime;

void TimeCache::update() {
	char buf[100];
	time_t now = time(0);
	struct tm* gmt = gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmt);
	_cachedTime = buf;
}

const std::string& TimeCache::getCurrentTime() {
	if (_cachedTime.empty()) {
		update();
	}
	return _cachedTime;
}
