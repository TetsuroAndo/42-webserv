/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 13:21:47 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/27 17:49:01 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

class CgiConfig
{
public: 
    std::string ext; // CGI スクリプトのファイル拡張子 (例: ".py", ".php")
    std::string bin; // CGI スクリプトを実行するためのバイナリパス (例: "/usr/bin/python3", "/usr/bin/php-cgi")
};

class LocationConfig
{
private:
   std::string                 _path;       // この設定が適用されるURLパス (例: "/", "/api", "/images")
    std::string                 _type;      // ロケーションのタイプ (例: "default", "redirect", "cgi") - カスタム実装に依存
    std::string                 _root;      // このロケーションで提供されるファイルのルートディレクトリパス (例: "./html")
    std::vector<std::string>    _index;     // ディレクトリリクエスト時に探すインデックスファイル名のリスト (例: "index.html", "index.htm")
    bool                        _autoindex; // ディレクトリリストの自動生成を有効にするか (true/false)
    std::vector<std::string>    _methods;   // このロケーションで許可されるHTTPメソッドのリスト (例: "GET", "POST", "DELETE")
    std::vector<CgiConfig>      _cgi;       // このロケーションでCGIを実行するための設定リスト
    std::string                 _redirect;  // このロケーションへのリクエストを転送する先のURL (HTTPリダイレクト用)

public:
	LocationConfig();
	~LocationConfig();

//setter
	void setPath(const std::string &path);
	void setType(const std::string &type);
	void setRoot(const std::string &root);
	void setIndex(const std::vector<std::string> &index);
	void setAutoindex(bool autoindex);
	void setMethods(const std::vector<std::string> &methods);
	void setCgi(const std::vector<CgiConfig> &cgi);
	void setRedirect(const std::string &redirect);

//helper
	void addMethod(const std::string &method);
	void addCgi(const CgiConfig &cgi_config);

//getter 
	const std::string &getPath() const;
	const std::string &getType() const;
	const std::string &getRoot() const;
	const std::vector<std::string> &getIndex() const;
	bool getAutoindex() const;
	const std::vector<std::string> &getMethods() const;
	const std::vector<CgiConfig> &getCgi() const;
	const std::string &getRedirect() const;

};