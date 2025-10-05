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
	static const std::string &getLocalTime();

private:
	static time_t _lastUpdateTime;
	static std::string _cachedGmtTime;
	static std::string _cachedLocalTime;

	TimeCache();
	TimeCache(const TimeCache &);
	TimeCache &operator=(const TimeCache &);
	~TimeCache();
};
