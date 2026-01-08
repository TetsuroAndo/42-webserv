#pragma once

#include "../../Config/Config.hpp"
#include <map>

namespace ServerBootstrap {

/// @brief Listen 構造体を人間が読みやすい文字列に変換
std::string listenToString(const Listen &listen);

/// @brief CGI 処理に使用する Config を選択して返します
/// @return CGI を処理するのに最も適した Config への参照
const Config &selectCgiConfig(const std::vector< Config > &configs);

/// @brief 複数の Config から同時に処理可能な最大イベント数を決定します
/// @return 設定に基づいて算出された最大イベント数（size_t）
size_t resolveMaxEvents(const std::vector< Config > &configs);

/// @brief  複数の Config を考慮してセッションの最大タイムアウトを決定します（秒）
/// @return セッションタイムアウトの最大値（秒）
size_t resolveMaxSessionTimeout(const std::vector< Config > &configs);

/// @brief 各 Listen に対して許容するリクエストヘッダの最大サイズを決定し、マップで返します
/// @return listen（文字列）をキー、ヘッダ最大サイズ（バイト）を値とするマップ
std::map< std::string, size_t >
resolveListenHeaderMax(const std::vector< Config > &configs);

/// @brief 複数の Config に定義された Listen 設定間の互換性を検証します
/// @throw std::runtime_error 等 互換性のない設定が見つかった場合に例外を投げる
void validateListenCompatibility(const std::vector< Config > &configs);

} // namespace ServerBootstrap
