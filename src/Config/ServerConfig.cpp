/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:10:58 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 15:17:59 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/ServerConfig.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

ServerConfig::ServerConfig()
	: _host("0.0.0.0"), _port(8080), _server_name("localhost"),
	  _client_max_body_size(1048576) {}

ServerConfig::~ServerConfig() {}

void ServerConfig::readFile(const std::string &filePath) {
	std::ifstream ifs(filePath.c_str());
	if (!ifs.is_open()) {
		std::string error_message = "エラー: ファイルを開けませんでした -> ";
		error_message += filePath;
		throw std::runtime_error(error_message);
	}
	//パースなどしていく 今は中身を出力しているだけ
	// std::stringstream buffer;
	// buffer << ifs.rdbuf();
	// std::string fileContent = buffer.str();
	// std::cout << "ファイルの内容\n" << fileContent << std::endl;
	ifs.close();
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