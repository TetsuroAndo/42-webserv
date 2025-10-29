#pragma once

#include <string>

class HttpResponse;
class Config;
struct Location;

namespace HandlerUtil {
std::string resolvePath(const std::string &requestPath, const Config &config,
						bool skipExistenceCheck = false);

/**
 * @brief リクエストパスからCGIスクリプトの仮想パスとPATH_INFOを抽出する
 *
 * 例: requestPath: /cgi-bin/echo.py/foo/bar, loc.path: /cgi-bin
 *   -> scriptVirtual: /cgi-bin/echo.py
 *   -> pathInfo: /foo/bar
 *
 * @param requestPath リクエストされたURIのパス部分（?より前）
 * @param loc 対象Location
 * @param[out] scriptVirtual Location基準のスクリプト仮想パス
 * @param[out] pathInfo スクリプト以降のPATH_INFO（先頭に'/'が付くか空文字）
 * @return 成功時true（locにマッチし、先頭セグメントが存在する場合）
 */
bool extractCgiScript(const std::string &requestPath, const Location &loc,
					  std::string &scriptVirtual, std::string &pathInfo);

std::string getDirName(const std::string &path);
} // namespace HandlerUtil
