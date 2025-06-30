/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 13:21:47 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 14:04:13 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>

/// @brief CGIのスクリプトの拡張子と実行するバイナリパス
///
/// @param ext CGI スクリプトのファイル拡張子 (例: ".py", ".php")
/// @param bin CGI スクリプトを実行するためのバイナリパス (例: "/usr/bin/python3", "/usr/bin/php-cgi")
struct CgiConfig
{
    std::string ext;
    std::string bin;
};

/// @brief このクラスは、ウェブサーバーの特定のURLパスに対する振る舞いを詳細に定義するための設定を保持するクラスです。これは、NGINX の設定ファイルにおける location ブロックに相当するものです。
///
/// @param _path      この設定が適用されるURLパス (例: "/", "/api", "/images")
/// @param _type      ロケーションのタイプ (例: "default", "redirect", "cgi") - カスタム実装に依存
/// @param _root      このロケーションで提供されるファイルのルートディレクトリパス (例: "./html")
/// @param _index     ディレクトリリクエスト時に探すインデックスファイル名のリスト (例: "index.html", "index.htm")
/// @param _autoindex ディレクトリリストの自動生成を有効にするか (true/false)
/// @param _methods   このロケーションで許可されるHTTPメソッドのリスト (例: "GET", "POST", "DELETE")
/// @param _cgi       このロケーションでCGIを実行するための設定リスト
/// @param _redirect  このロケーションへのリクエストを転送する先のURL (HTTPリダイレクト用)
class LocationConfig
{
private:
   std::string                 _path;
    std::string                 _type;
    std::string                 _root;
    std::vector<std::string>    _index;
    bool                        _autoindex;
    std::vector<std::string>    _methods;
    std::vector<CgiConfig>      _cgi;
    std::string                 _redirect;
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