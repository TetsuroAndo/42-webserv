/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:10:58 by hirwatan          #+#    #+#             */
/*   Updated: 2025/07/01 07:51:17 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/ServerConfig.hpp"
#include "../../inc/Utils.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

ServerConfig::ServerConfig(){}

ServerConfig::~ServerConfig(){}

std::vector<ServerConfig> ServerConfig::parseConfigFile(const std::string &filePath) {
    std::ifstream ifs(filePath.c_str());
    if (!ifs.is_open()) {
        std::string error_message = "エラー: ファイルを開けませんでした -> ";
        error_message += filePath;
        throw std::runtime_error(error_message);
    }
    
    std::vector<ServerConfig> servers;
    std::string line;
    bool inServerBlock = false;
    bool inLocationBlock = false;
    ServerConfig currentServer;
    LocationConfig currentLocation;

    while (std::getline(ifs, line)) {
        
        // コメントと空白スキップ
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        
        line = ft_trim(line);
        
        if (line.empty()) continue;

        // server {が存在するかどうか
        if(line.find("server {") != std::string::npos) {
            if (inServerBlock) {
                // 前のサーバーを保存
                servers.push_back(currentServer);
            }
            currentServer = ServerConfig();  // 新しいサーバー設定を開始
            inServerBlock = true;
            inLocationBlock = false;
            continue;
        }

        if (inServerBlock && line == "}") {
            if (inLocationBlock) {
                currentServer.addLocation(currentLocation);
                currentLocation = LocationConfig();
                inLocationBlock = false;
            } else {
                // サーバーブロックの終了
                servers.push_back(currentServer);
                inServerBlock = false;
            }
            continue;
        }
        
        if (inServerBlock) {
            if (line.find("location ") != std::string::npos) {
                if (inLocationBlock) {
                    currentServer.addLocation(currentLocation);
                }
                currentLocation = LocationConfig();
                inLocationBlock = true;
                
                // location パスを抽出
                size_t start = line.find("location ") + 9;
                size_t end = line.find(" {");
                if (end != std::string::npos) {
                    std::string path = line.substr(start, end - start);
                    path = ft_trim(path);
                    currentLocation.setPath(path);
                }
                continue;
            }
            
            if (inLocationBlock) {
                currentServer.parseLocationLine(line, currentLocation);
            } else {
                currentServer.parseServerLine(line);
            }
        }
    }
    
    // 最後のサーバーまたはロケーションを追加
    if (inLocationBlock) {
        currentServer.addLocation(currentLocation);
    }
    if (inServerBlock) {
        servers.push_back(currentServer);
    }
    
    ifs.close();
    return servers;
}

void ServerConfig::parseServerLine(const std::string &line) {
    std::istringstream iss(line);
    std::string directive;
    iss >> directive;
    
    if (directive == "listen") {
        std::string listen_value;
        iss >> listen_value;
        
        // ":"で分割してホストとポートを取得
        size_t colon_pos = listen_value.find(':');
        if (colon_pos != std::string::npos) {
            _host = listen_value.substr(0, colon_pos);
            _port = atoi(listen_value.substr(colon_pos + 1).c_str());
        } else {
            _port = atoi(listen_value.c_str());
        }
    }
    else if (directive == "server_name") {
        std::string server_name;
        iss >> server_name;
        server_name = ft_trimsemicolon(server_name);
        _server_name = server_name;
    }
    else if (directive == "client_max_body_size") {
        std::string size_str;
        iss >> size_str;
        
        if (!size_str.empty()) {
            char last_char = size_str[size_str.length() - 1];
            if (last_char == 'm' || last_char == 'M') {
                size_str = size_str.substr(0, size_str.length() - 1);
                _client_max_body_size = atol(size_str.c_str()) * 1024 * 1024;
            } else if (last_char == 'k' || last_char == 'K') {
                size_str = size_str.substr(0, size_str.length() - 1);
                _client_max_body_size = atol(size_str.c_str()) * 1024;
            } else {
                _client_max_body_size = atol(size_str.c_str());
            }
        }
    }
    else if (directive == "error_page") {
        int status_code;
        std::string path;
        iss >> status_code >> path;
        path = ft_trimsemicolon(path);
        addErrorPage(status_code, path);
    }
}

void ServerConfig::parseLocationLine(const std::string &line, LocationConfig &location) {
    std::istringstream iss(line);
    std::string directive;
    iss >> directive;

    if (directive == "root") {
        std::string root;
        iss >> root;
        root = ft_trimsemicolon(root);
        location.setRoot(root);
    }
    else if (directive == "index") {
        std::vector<std::string> indices;
        std::string index;
        while (iss >> index) {
            indices.push_back(index);
        }
        location.setIndex(indices);
    }
    else if (directive == "autoindex") {
        std::string value;
        iss >> value;
        location.setAutoindex(value == "on");
    }
    else if (directive == "methods") {
        std::vector<std::string> methods;
        std::string method;
        while (iss >> method) {
            methods.push_back(method);
        }
        location.setMethods(methods);
    }
    else if (directive == "cgi") {
        std::string ext, bin;
        iss >> ext >> bin;
        CgiConfig cgi_config;
        cgi_config.ext = ext;
        cgi_config.bin = bin;
        location.addCgi(cgi_config);
    }
    else if (directive == "redirect") {
        std::string redirect_url;
        iss >> redirect_url;
        location.setRedirect(redirect_url);
    }
}


//setter
void ServerConfig::setHost(const std::string &host) { _host = host; }
void ServerConfig::setPort(int port) { _port = port; }
void ServerConfig::setServerName(const std::string &server_name) { _server_name = server_name; }
void ServerConfig::setClientMaxBodySize(long client_max_body_size) { _client_max_body_size = client_max_body_size; }
void ServerConfig::setErrorPages(const std::map<int, std::string> &error_pages) { _error_pages = error_pages; }
void ServerConfig::setLocations(const std::vector<LocationConfig> &locations) { _locations = locations; }

//helper
void ServerConfig::addErrorPage(int status_code, const std::string &path) { _error_pages[status_code] = path; }
void ServerConfig::addLocation(const LocationConfig &location_config) { _locations.push_back(location_config); }

//getter
const std::string &ServerConfig::getHost() const { return _host; }
int ServerConfig::getPort() const { return _port; }
const std::string &ServerConfig::getServerName() const { return _server_name; }
long ServerConfig::getClientMaxBodySize() const { return _client_max_body_size; }
const std::map<int, std::string> &ServerConfig::getErrorPages() const { return _error_pages; }
const std::vector<LocationConfig> &ServerConfig::getLocations() const { return _locations; }