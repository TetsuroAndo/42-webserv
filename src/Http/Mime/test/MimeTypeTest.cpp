#include "../MimeType.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

// 既知の拡張子テスト
static void testKnownExtensions() {
    typedef std::pair<std::string, std::string> StrPair;
    std::vector<StrPair> cases;
    cases.push_back(StrPair(".html", "text/html"));
    cases.push_back(StrPair(".css", "text/css"));
    cases.push_back(StrPair(".js", "application/javascript"));
    cases.push_back(StrPair(".json", "application/json"));
    cases.push_back(StrPair(".xml", "application/xml"));
    cases.push_back(StrPair(".pdf", "application/pdf"));
    cases.push_back(StrPair(".zip", "application/zip"));
    cases.push_back(StrPair(".txt", "text/plain"));
    cases.push_back(StrPair(".jpeg", "image/jpeg"));
    cases.push_back(StrPair(".jpg", "image/jpeg"));
    cases.push_back(StrPair(".png", "image/png"));
    cases.push_back(StrPair(".gif", "image/gif"));
    cases.push_back(StrPair(".bmp", "image/bmp"));
    cases.push_back(StrPair(".ico", "image/x-icon"));
    cases.push_back(StrPair(".svg", "image/svg+xml"));
    cases.push_back(StrPair(".mp3", "audio/mpeg"));
    cases.push_back(StrPair(".mp4", "video/mp4"));
    cases.push_back(StrPair(".webm", "video/webm"));
    cases.push_back(StrPair(".wav", "audio/wav"));
    cases.push_back(StrPair(".ttf", "font/ttf"));
    cases.push_back(StrPair(".otf", "font/otf"));
    cases.push_back(StrPair(".woff", "font/woff"));
    cases.push_back(StrPair(".woff2", "font/woff2"));
    cases.push_back(StrPair(".eot", "application/vnd.ms-fontobject"));

    for (size_t i = 0; i < cases.size(); ++i) {
        std::string result = MimeType::getMimeType("file" + cases[i].first);
        if (result == cases[i].second) {
            std::cout << "\033[32m[PASS]\033[0m " << cases[i].first << std::endl;
        } else {
            std::cout << "\033[31m[FAIL]\033[0m " << cases[i].first
                      << " (got '" << result << "')" << std::endl;
        }
    }
}

// ランダム拡張子テスト
static void testRandomExtensions(size_t n) {
    std::srand((unsigned int)std::time(NULL));
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (size_t i = 0; i < n; ++i) {
        size_t len = 1 + std::rand() % 10;
        std::string ext = ".";
        for (size_t j = 0; j < len; ++j) {
            ext += chars[std::rand() % chars.size()];
        }
        std::string result = MimeType::getMimeType("file" + ext);
        if (result == "application/octet-stream") {
            std::cout << "\033[32m[PASS]\033[0m Random " << ext << std::endl;
        } else {
            std::cout << "\033[31m[FAIL]\033[0m Random " << ext
                      << " (unexpected '" << result << "')" << std::endl;
        }
    }
}

// エッジケーステスト
static void testEdgeCases() {
    typedef std::pair<std::string, std::string> StrPair;
    std::vector<StrPair> edgeCases;
    edgeCases.push_back(StrPair("", "application/octet-stream"));
    edgeCases.push_back(StrPair(".", "application/octet-stream"));
    edgeCases.push_back(StrPair(".HTML", "text/html"));
    edgeCases.push_back(StrPair(".Js", "application/javascript"));
    edgeCases.push_back(StrPair(".UNKNOWNEXT", "application/octet-stream"));

    for (size_t i = 0; i < edgeCases.size(); ++i) {
        std::string result = MimeType::getMimeType(edgeCases[i].first);
        if (result == edgeCases[i].second) {
            std::cout << "\033[32m[PASS]\033[0m " << edgeCases[i].first << std::endl;
        } else {
            std::cout << "\033[31m[FAIL]\033[0m " << edgeCases[i].first
                      << " (got '" << result << "')" << std::endl;
        }
    }
}

int main() {
    std::cout << "=== MimeType Comprehensive Tests ===" << std::endl;
    testKnownExtensions();
    testRandomExtensions(200); // ランダム 200件
    testEdgeCases();
    return 0;
}
