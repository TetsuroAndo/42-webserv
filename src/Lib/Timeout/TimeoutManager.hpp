#pragma once

#include <ctime>
#include <map>

class ITimeoutable;

class TimeoutManager {
public:
	TimeoutManager();
	~TimeoutManager();

	/**
	 * @brief タイムアウト監視対象を追加または更新します。
	 * @param obj 監視対象オブジェクトへのポインタ。
	 * @param timeoutSec 現在時刻からのタイムアウト秒数。
	 */
	void add(ITimeoutable *obj, time_t timeoutSec);

	/**
	 * @brief タイムアウト監視対象を削除します。
	 * @param obj 監視対象オブジェクトへのポインタ。
	 */
	void remove(ITimeoutable *obj);

	/**
	 * @brief
	 * タイムアウトした全てのオブジェクトをチェックし、onTimeout()を呼び出します。
	 */
	void checkAndHandleTimeouts();

	/**
	 * @brief 次に発生するタイムアウトまでの時間をミリ秒で取得します。
	 * epoll_waitなどのタイムアウト値として使用します。
	 * @return 次のタイムアウトまでのミリ秒。監視対象がなければ-1。
	 */
	int getNextTimeoutInterval() const;

private:
	TimeoutManager(const TimeoutManager &);
	TimeoutManager &operator=(const TimeoutManager &);

	// タイムアウト時刻をキーとしてオブジェクトを管理するマップ
	typedef std::map< time_t, ITimeoutable * > TimeoutMap;
	// オブジェクトからタイムアウト時刻を逆引きするためのマップ (削除を高速化)
	typedef std::map< ITimeoutable *, time_t > ReverseTimeoutMap;

	TimeoutMap _timeoutMap;
	ReverseTimeoutMap _reverseMap;
};
