/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:10:53 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 17:22:58 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include <iostream>

Webserv::Webserv(const std::string &filePath) {
    _serverconfigs = ServerConfig::parseConfigFile(filePath);
    printConfigs();
}

Webserv::~Webserv() {}

const std::vector<ServerConfig>& Webserv::getServerConfigs() const {
    return _serverconfigs;
}

void Webserv::printConfigs() const {
    std::cout << "=== Loaded " << _serverconfigs.size() << " server configurations ===" << std::endl;
    for (size_t i = 0; i < _serverconfigs.size(); ++i) {
        std::cout << "--- Server " << (i + 1) << " ---" << std::endl;
        std::cout << _serverconfigs[i] << std::endl;
    }
}

void Webserv::run() {
    std::cout << "Starting " << _serverconfigs.size() << " servers..." << std::endl;
}
//debug
std::ostream &operator<<(std::ostream &os, const ServerConfig &config)
{
    os << "=== Server Configuration ===" << std::endl;
    os << "Host: " << config.getHost() << std::endl;
    os << "Port: " << config.getPort() << std::endl;
    os << "Server Name: " << config.getServerName() << std::endl;
    os << "Client Max Body Size: " << config.getClientMaxBodySize() << std::endl;
    
    // Error Pages
    const std::map<int, std::string> &errorPages = config.getErrorPages();
    if (!errorPages.empty()) {
        os << "Error Pages:" << std::endl;
        for (std::map<int, std::string>::const_iterator it = errorPages.begin(); 
             it != errorPages.end(); ++it) {
            os << "  " << it->first << ": " << it->second << std::endl;
        }
    }
    
    // Locations
    const std::vector<LocationConfig> &locations = config.getLocations();
    if (!locations.empty()) {
        os << "Locations:" << std::endl;
        for (size_t i = 0; i < locations.size(); ++i) {
            os << "  Path: " << locations[i].getPath() << std::endl;
            os << "  Root: " << locations[i].getRoot() << std::endl;
            os << "  Autoindex: " << (locations[i].getAutoindex() ? "on" : "off") << std::endl;
        }
    }
    
    return os;
}