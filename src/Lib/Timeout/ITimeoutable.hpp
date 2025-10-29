#pragma once

class ITimeoutable {
public:
	virtual ~ITimeoutable() {}

	/**
	 * @brief タイムアウト時に呼び出されるコールバック関数。
	 */
	virtual void onTimeout() = 0;
};
