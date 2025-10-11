#include "TimeCache.hpp"
#include "TimeFormatter.hpp"
#include <ctime>

time_t TimeCache::_lastUpdateTime = 0;
std::string TimeCache::_cachedGmtTime;
std::string TimeCache::_cachedGmtDate;
std::string TimeCache::_cachedLocalTime;
std::string TimeCache::_cachedLocalDate;
std::string TimeCache::_cachedHeaderTimestamp;
std::string TimeCache::_cachedLocalTimestamp;
std::string TimeCache::_cachedUtcTimestamp;
std::string TimeCache::_cachedIsoTimestamp;

void TimeCache::update() {
	const time_t now = time(0);
	if(now == _lastUpdateTime) {
		return;
	}
	_lastUpdateTime = now;

	struct tm gmt;
	if (gmtime_r(&now, &gmt) == NULL) {
		_cachedGmtTime = "00:00:00";
		_cachedGmtDate = "1970-01-01";
		_cachedUtcTimestamp = "1970-01-01 00:00:00Z";
		_cachedIsoTimestamp = "1970-01-01T00:00:00Z";
		_cachedHeaderTimestamp = "Thu, 01 Jan 1970 00:00:00 GMT";
	} else {
		TimeFormatter::getTime(_cachedGmtTime, gmt);
		TimeFormatter::getDate(_cachedGmtDate, gmt);
		TimeFormatter::getUtcTimestamp(_cachedUtcTimestamp, gmt);
		TimeFormatter::getIsoTimestamp(_cachedIsoTimestamp, gmt);
		TimeFormatter::getHeaderTimestamp(_cachedHeaderTimestamp, gmt);
	}

	struct tm local;
	if (localtime_r(&now, &local) == NULL) {
		_cachedLocalTime = "00:00:00";
		_cachedLocalDate = "1970-01-01";
		_cachedLocalTimestamp = "1970-01-01 00:00:00";
	} else {
		TimeFormatter::getTime(_cachedLocalTime, local);
		TimeFormatter::getDate(_cachedLocalDate, local);
		TimeFormatter::getLocalTimestamp(_cachedLocalTimestamp, local);
	}
}

const std::string &TimeCache::getGmtTime() {
	update();
	return _cachedGmtTime;
}

const std::string &TimeCache::getGmtDate() {
	update();
	return _cachedGmtDate;
}

const std::string &TimeCache::getLocalTime() {
	update();
	return _cachedLocalTime;
}

const std::string &TimeCache::getLocalDate() {
	update();
	return _cachedLocalDate;
}

const std::string &TimeCache::getLocalTimestamp() {
	update();
	return _cachedLocalTimestamp;
}

const std::string &TimeCache::getUtcTimestamp() {
	update();
	return _cachedUtcTimestamp;
}

const std::string &TimeCache::getIsoTimestamp() {
	update();
	return _cachedIsoTimestamp;
}

const std::string &TimeCache::getHeaderTimestamp() {
	update();
	return _cachedHeaderTimestamp;
}
