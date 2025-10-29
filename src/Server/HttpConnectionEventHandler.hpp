#pragma once

/**
 * @class HttpConnectionEventHandler
 * @brief HttpConnectionからClientへのイベント通知用インターフェース
 */
class HttpConnectionEventHandler {
public:
	virtual ~HttpConnectionEventHandler() {}

	/// @brief 接続クローズ通知
	virtual void onConnectionClose(int fd) = 0;

	/// @brief ソケットイベント変更通知
	virtual void onSocketModify(int fd, uint32_t events) = 0;

	/// @brief CGI関連の変更通知
	virtual void onCgiChanges() = 0;

	/// @brief リクエスト処理完了通知
	virtual void onRequestProcessed() = 0;
};
