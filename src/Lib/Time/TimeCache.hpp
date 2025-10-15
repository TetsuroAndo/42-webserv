#pragma once

#include <ctime>
#include <string>

class TimeCache {
public:
	/**
	 * @brief 現在時刻のキャッシュを更新します。
	 * 1秒に1回程度の頻度でServerなどのLoop処理内で、呼び出すことを想定しています。
	 */
	static void update();

	/**
	 * @brief キャッシュされた現在時刻の文字列を取得します。
	 * キャッシュが古い場合は更新されます。
	 * @return キャッシュされた現在時刻の文字列
	 */
	static const std::string &getGmtTime();
	static const std::string &getGmtDate();
	static const std::string &getLocalTime();
	static const std::string &getLocalDate();
	static const std::string &getLocalTimestamp();
	static const std::string &getIsoTimestamp();
	static const std::string &getUtcTimestamp();
	static const std::string &getHeaderTimestamp();

private:
	static time_t _lastUpdateTime;
	static std::string _cachedGmtTime;
	static std::string _cachedGmtDate;
	static std::string _cachedLocalTime;
	static std::string _cachedLocalDate;
	static std::string _cachedHeaderTimestamp;
	static std::string _cachedLocalTimestamp;
	static std::string _cachedUtcTimestamp;
	static std::string _cachedIsoTimestamp;

	TimeCache();
	TimeCache(const TimeCache &);
	TimeCache &operator=(const TimeCache &);
	~TimeCache();
};
