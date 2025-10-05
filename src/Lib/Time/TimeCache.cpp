#include "TimeCache.hpp"
#include <ctime>

void TimeCache::update() {
	const time_t now = time(0);
	if(now == _lastUpdateTime) {
		return;
	}
	_lastUpdateTime = now;

	struct tm gmt;
	gmtime_r(&now, &gmt);
	char gmtBuf[64];
	strftime(gmtBuf, sizeof(gmtBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt);
	_cachedGmtTime = gmtBuf;

	struct tm local;
	localtime_r(&now, &local);
	char localBuf[64];
	strftime(localBuf, sizeof(localBuf), "%Y-%m-%d %H:%M:%S %Z", &local);
	_cachedLocalTime = localBuf;
}

const std::string &TimeCache::getGmtTime() {
	update();
	return _cachedGmtTime;
}

const std::string &TimeCache::getLocalTime() {
	update();
	return _cachedLocalTime;
}
