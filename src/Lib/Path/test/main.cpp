#include "../Path.hpp"
#include <iostream>
#include <cassert>
#include <stdexcept> // For std::runtime_error

// アサーション失敗時にメッセージを表示するためのマクロ
#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "Assertion failed: \n" \
                  << "  File: " << __FILE__ << "\n" \
                  << "  Line: " << __LINE__ << "\n" \
                  << "  Expected: \"" << (expected) << "\"\n" \
                  << "  Actual:   \"" << (actual) << "\"\n" \
                  << "  Path:     \"" << path << "\"\n" \
                  << std::endl; \
        throw std::runtime_error("Assertion failed"); \
    }

void test_normalize() {
    std::cout << "Testing normalize..." << std::endl;

    std::string path; // For logging in ASSERT_EQ

    // 基本ケース
    path = "/a/b/c"; ASSERT_EQ("/a/b/c", Path::normalize(path));
    path = "/a/b/../c"; ASSERT_EQ("/a/c", Path::normalize(path));
    path = "/a/./b/c/"; ASSERT_EQ("/a/b/c", Path::normalize(path));
    path = "//a//b"; ASSERT_EQ("/a/b", Path::normalize(path));

    // エッジケース
    path = "/"; ASSERT_EQ("/", Path::normalize(path));
    path = "."; ASSERT_EQ(".", Path::normalize(path));
    path = ".."; ASSERT_EQ("..", Path::normalize(path)); // 相対パスの".."はそのまま
    path = "/.."; ASSERT_EQ("/", Path::normalize(path));
    path = "/a/../../b"; ASSERT_EQ("/b", Path::normalize(path));
    path = "a/../b"; ASSERT_EQ("b", Path::normalize(path));
    path = ""; ASSERT_EQ(".", Path::normalize(path));
    path = "a/b/../../.."; ASSERT_EQ("..", Path::normalize(path)); // "a/b" -> "a" -> "" -> ".."
    path = "../../a"; ASSERT_EQ("../../a", Path::normalize(path));
    path = "/a/b/.."; ASSERT_EQ("/a", Path::normalize(path));
    path = "a/b/../.."; ASSERT_EQ(".", Path::normalize(path)); // "a/b" -> "a" -> "" -> "."

    // ユーザーが指摘したケース
    // 複数 ".." と "." 混在/a/./b/.././../cpop/push の両方起きる複合ケース
    path = "/a/./b/.././../cpop"; ASSERT_EQ("/cpop", Path::normalize(path));

    // ////（ルート直下の複数スラッシュ）////a////b最初のスラッシュ連続時の動作確認
    path = "////a////b"; ASSERT_EQ("/a/b", Path::normalize(path));

    // 最後が ".." で終わる/a/b/.. や a/..末尾 ".." の扱い（/a/b/..→/a）を確認
    path = "/a/b/.."; ASSERT_EQ("/a", Path::normalize(path));
    path = "a/.."; ASSERT_EQ(".", Path::normalize(path)); // a -> "" -> "."

    // "../" だけのケース"../" → ".."空ではなく ".." のまま残るか
    path = "../"; ASSERT_EQ("..", Path::normalize(path));

    // "./../a""./../a" → "../a""." + ".." の順序混在確認
    path = "./../a"; ASSERT_EQ("../a", Path::normalize(path));

    // "." のみで末尾スラッシュ
    path = "./"; ASSERT_EQ(".", Path::normalize(path));

    std::cout << "normalize tests passed!" << std::endl;
}

void test_getAbsolutePath() {
    std::cout << "Testing getAbsolutePath..." << std::endl;
    
    // このテストは環境に依存する
    // 存在するファイルやディレクトリでテストする
    std::string cwd = Path::getAbsolutePath(".");
    assert(!cwd.empty());
    std::cout << "Current dir: " << cwd << std::endl;

    // 存在しないパスのテスト
    assert(Path::getAbsolutePath("/path/to/non/existent/file") == ""); // Use direct assert

    std::cout << "getAbsolutePath tests passed!" << std::endl;
}


int main() {
    try {
        test_normalize();
        test_getAbsolutePath();
        std::cout << "\nAll Path tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\n" << e.what() << std::endl;
        return 1;
    }
    return 0;
}
