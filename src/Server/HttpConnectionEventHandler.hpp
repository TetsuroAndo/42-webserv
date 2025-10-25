#pragma once

/**
 * @class HttpConnectionEventHandler
 * @brief HttpConnectionからClientへのイベント通知用インターフェース
 */
class HttpConnectionEventHandler {
public:
    virtual ~HttpConnectionEventHandler() {}
    virtual void onConnectionClose(int fd) = 0;
    virtual void onSocketModify(int fd, uint32_t events) = 0;
    virtual void onCgiChanges() = 0;
    virtual void onRequestProcessed() = 0;
};
