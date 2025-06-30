/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 14:33:01 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 14:37:34 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/LocationConfig.hpp"

// コンストラクタ
LocationConfig::LocationConfig()
	: _path("/"), _type("default"), _root("./html"), _autoindex(false) {
	_index.push_back("index.html");
	_methods.push_back("GET");
}

LocationConfig::~LocationConfig() {}

//setter
void LocationConfig::setPath(const std::string &path) { _path = path; }
void LocationConfig::setType(const std::string &type) { _type = type; }
void LocationConfig::setRoot(const std::string &root) { _root = root; }
void LocationConfig::setIndex(const std::vector<std::string> &index) { _index = index; }
void LocationConfig::setAutoindex(bool autoindex) { _autoindex = autoindex; }
void LocationConfig::setMethods(const std::vector<std::string> &methods) { _methods = methods; }
void LocationConfig::setCgi(const std::vector<CgiConfig> &cgi) { _cgi = cgi; }
void LocationConfig::setRedirect(const std::string &redirect) { _redirect = redirect; }

//helper
void LocationConfig::addMethod(const std::string &method) { _methods.push_back(method); }
void LocationConfig::addCgi(const CgiConfig &cgi_config) { _cgi.push_back(cgi_config); }

//getter
const std::string &LocationConfig::getPath() const { return _path; }
const std::string &LocationConfig::getType() const { return _type; }
const std::string &LocationConfig::getRoot() const { return _root; }
const std::vector<std::string> &LocationConfig::getIndex() const { return _index; }
bool LocationConfig::getAutoindex() const { return _autoindex; }
const std::vector<std::string> &LocationConfig::getMethods() const { return _methods; }
const std::vector<CgiConfig> &LocationConfig::getCgi() const { return _cgi; }
const std::string &LocationConfig::getRedirect() const { return _redirect; }