#pragma once

#include <ctime>
#include <map>
#include <list>

class ITimeoutable;

/**
 * @class TimeoutStats
 * @brief タイムアウト統計情報を管理するクラス
 */
class TimeoutStats {
public:
    TimeoutStats() : _totalTimeouts(0), _activeConnections(0), _maxActiveConnections(0) {}

    void incrementTimeouts() { ++_totalTimeouts; }
    void incrementActiveConnections() {
        ++_activeConnections;
        if (_activeConnections > _maxActiveConnections) {
            _maxActiveConnections = _activeConnections;
        }
    }
    void decrementActiveConnections() { --_activeConnections; }

    size_t getTotalTimeouts() const { return _totalTimeouts; }
    size_t getActiveConnections() const { return _activeConnections; }
    size_t getMaxActiveConnections() const { return _maxActiveConnections; }

    void reset() {
        _totalTimeouts = 0;
        _activeConnections = 0;
        _maxActiveConnections = 0;
    }

private:
    size_t _totalTimeouts;
    size_t _activeConnections;
    size_t _maxActiveConnections;
};

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

	/**
	 * @brief 現在監視中のオブジェクト数を取得します。
	 */
	size_t getActiveTimeoutCount() const;

	/**
	 * @brief 統計情報を取得します。
	 */
	const TimeoutStats& getStats() const { return _stats; }

private:
	TimeoutManager(const TimeoutManager &);
	TimeoutManager &operator=(const TimeoutManager &);

	// タイムアウト時刻をキーとしてオブジェクトリストを管理するマップ
	typedef std::map< time_t, std::list<ITimeoutable*> > TimeoutMap;
	// オブジェクトからタイムアウト時刻を逆引きするためのマップ (削除を高速化)
	typedef std::map< ITimeoutable *, time_t > ReverseTimeoutMap;

	TimeoutMap _timeoutMap;
	ReverseTimeoutMap _reverseMap;
	TimeoutStats _stats;
};
