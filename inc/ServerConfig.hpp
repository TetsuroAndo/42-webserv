/* ************************************************************************** */
/* */
/* :::      ::::::::   */
/* ServerConfig.hpp                                   :+:      :+:    :+:   */
/* +:+ +:+         +:+     */
/* By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/* +#+#+#+#+#+   +#+           */
/* Created: 2025/06/27 13:21:50 by hirwatan          #+#    #+#             */
/* Updated: 2025/06/27 17:52:35 by hirwatan         ###   ########.fr       */
/* */
/* ************************************************************************** */

#pragma once // ヘッダーファイルの多重インクルードを防ぐ

#include "LocationConfig.hpp" // LocationConfig クラスの定義を使用するためにインクルード
#include <map>				  // std::map を使用するためにインクルード
#include <string>			  // std::string を使用するためにインクルード
#include <vector>			  // std::vector を使用するためにインクルード

// ウェブサーバー全体のグローバルな設定、または特定の「サーバーブロック」の設定を保持するクラス。
// NGINX の 'server'
// ブロックに相当し、特定のホスト名とポートでリッスンするサーバーの挙動を定義します。
class ServerConfig {
private:
	std::string
		host; // サーバーがリッスンするIPアドレス (例: "0.0.0.0", "127.0.0.1")
	int port; // サーバーがリッスンするTCPポート番号 (例: 80, 8080)
	std::string
		server_name; // このサーバーブロックが応答するホスト名 (例:
					 // "example.com", "localhost")
					 // ホストヘッダーと一致した場合にこの設定が適用されます。
	long client_max_body_size; // クライアントからのリクエストボディの最大許容サイズ
							   // (バイト単位)。 これを超えると通常 413 Request
							   // Entity Too Large エラーを返します。
	std::map<int, std::string>
		error_pages; // カスタムエラーページの設定。
					 // キーはHTTPステータスコード (例: 404,
					 // 500)、値は対応するエラーページのパス (例:
					 // "/errors/404.html")。
	std::vector<LocationConfig>
		locations; // このサーバーブロック内で定義される複数のロケーション設定のリスト。
				   // 各 LocationConfig
				   // は特定のURLパスに対する振る舞いを定義します。
public:
	// デフォルトコンストラクタ
	ServerConfig();
	// デストラクタ
	~ServerConfig();

	// セッター (setter) メソッド群: private
	// メンバー変数の値を設定するために使用
	void setHost(const std::string &host);
	void setPort(int port);
	void setServerName(const std::string &server_name);
	void setClientMaxBodySize(long client_max_body_size);
	void setErrorPages(const std::map<int, std::string> &error_pages);
	void setLocations(const std::vector<LocationConfig> &locations);

	// ヘルパーメソッド: error_pages に個別のエラーページを追加
	void addErrorPage(int status_code, const std::string &path);
	// ヘルパーメソッド: locations に個別の LocationConfig を追加
	void addLocation(const LocationConfig &location_config);

	// ゲッター (getter) メソッド群: private
	// メンバー変数の値を取得するために使用 (const
	// 修飾子により、取得した値の変更を禁止)
	const std::string &getHost() const;
	int getPort() const;
	const std::string &getServerName() const;
	long getClientMaxBodySize() const;
	const std::map<int, std::string> &getErrorPages() const;
	const std::vector<LocationConfig> &getLocations() const;
};