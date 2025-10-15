#pragma once
#include <string>

namespace Path {
// realpathのC++ラッパー。パスが存在しない場合は空文字を返す。
std::string getAbsolutePath(const std::string &path);

// 文字列操作ベースのパス正規化。パスの存在チェックは不要。
std::string normalize(const std::string &path);
} // namespace Path
