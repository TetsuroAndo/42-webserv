/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 14:28:13 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 17:20:44 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "LocationConfig.hpp"
#include <map>
#include <string>
#include <vector>

/// @brief ウェブサーバー全体のグローバルな設定、または特定の「サーバーブロック」の設定を保持するクラス。
/// @details NGINX の 'server' ブロックに相当し、特定のホスト名とポートでリッスンするサーバーの挙動を定義します。
/// @param _host サーバーがリッスンするIPアドレス (例: "0.0.0.0", "127.0.0.1")
/// @param _port サーバーがリッスンするTCPポート番号 (例: 80, 8080)
/// @param _server_name このサーバーブロックが応答するホスト名 (例:"example.com", "localhost")ホストヘッダーと一致した場合にこの設定が適用されます
/// @param _client_max_body_size クライアントからのリクエストボディの最大許容サイズ  (バイト単位)。 これを超えると通常 413 Request Entity Too Large エラーを返します。
/// @param _error_pages カスタムエラーページの設定。 キーはHTTPステータスコード (例: 404,500)、値は対応するエラーページのパス (例:"/errors/404.html")。
/// @param _locations  このサーバーブロック内で定義される複数のロケーション設定のリスト。各 LocationConfigは特定のURLパスに対する振る舞いを定義します。
class ServerConfig 
{
private:
    std::string _host; 
    int _port;
    std::string _server_name;
    long _client_max_body_size;
    std::map<int, std::string> _error_pages;
    std::vector<LocationConfig> _locations;
public:
    ServerConfig();
    ~ServerConfig();

//setter
    void setHost(const std::string &host);
    void setPort(int port);
    void setServerName(const std::string &server_name);
    void setClientMaxBodySize(long client_max_body_size);
    void setErrorPages(const std::map<int, std::string> &error_pages);
    void setLocations(const std::vector<LocationConfig> &locations);

//helper
    void addErrorPage(int status_code, const std::string &path);
    void addLocation(const LocationConfig &location_config);

//getter
    const std::string &getHost() const;
    int getPort() const;
    const std::string &getServerName() const;
    long getClientMaxBodySize() const;
    const std::map<int, std::string> &getErrorPages() const;
    const std::vector<LocationConfig> &getLocations() const;

//parse
    static std::vector<ServerConfig> parseConfigFile(const std::string &filePath);
    void parseServerLine(const std::string &line);
    void parseLocationLine(const std::string &line, LocationConfig &location);

};