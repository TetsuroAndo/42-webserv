#pragma once

#include <unistd.h>

namespace Write {

/**
 * @brief writeのラッパー関数。戻り値を適切に消費し、EINTR
 * を考慮して書き込みを試みる。
 * @note それ以外のエラーは諦める。
 * @param fd 書き込み先のFD
 * @param data 書き込みデータ
 * @param len 書き込みデータの長さ
 */
void xwrite(int fd, const char *data, size_t len);
} // namespace Write
